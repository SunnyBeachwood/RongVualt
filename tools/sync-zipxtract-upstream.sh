#!/usr/bin/env bash
set -euo pipefail
export LC_ALL=C

# Refresh only the read-only mirror. Adapter code and PORT_MAP.json are never
# copied from upstream. Run from the RongVualt repository root.
APPLY=0
if [ "${1:-}" = "--apply" ]; then
  APPLY=1
  shift
fi
REF="${1:-v7.1.1}"
EXPECTED_COMMIT="${ZIPXTRACT_EXPECTED_COMMIT:-}"
# Keep the checked-in baseline immutable even if a remote tag is ever moved;
# callers syncing a newer ref can provide its resolved commit explicitly.
if [ "$REF" = "v7.1.1" ] && [ -z "$EXPECTED_COMMIT" ]; then
  EXPECTED_COMMIT="c137bc1e07e8bea89f13dc26a938fd35df62918c"
fi
ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
DEST="$ROOT_DIR/third_party/zipxtract"
WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/rongvault-zipxtract.XXXXXX")"
trap 'rm -rf "$WORK_DIR"' EXIT

git clone --filter=blob:none --no-checkout https://github.com/WirelessAlien/ZipXtract "$WORK_DIR/repo"
git -C "$WORK_DIR/repo" fetch --depth=1 origin "$REF"
COMMIT="$(git -C "$WORK_DIR/repo" rev-parse "FETCH_HEAD^{commit}")"
if [ -n "$EXPECTED_COMMIT" ] && [ "$COMMIT" != "$EXPECTED_COMMIT" ]; then
  echo "resolved $REF to $COMMIT, expected $EXPECTED_COMMIT" >&2
  exit 2
fi

mkdir "$WORK_DIR/snapshot"
git -C "$WORK_DIR/repo" archive "$COMMIT" | tar -x -C "$WORK_DIR/snapshot"
(
  cd "$WORK_DIR/snapshot"
  find . -type f \
    ! -name SOURCE_MANIFEST.sha256 ! -name UPSTREAM.md ! -name PORT_MAP.json \
    -printf '%P\0' | sort -z | xargs -0 sha256sum
) > "$WORK_DIR/SOURCE_MANIFEST.sha256"
printf 'resolved %s -> %s\n' "$REF" "$COMMIT"
MAPPED_CHANGED=0
if [ -f "$DEST/PORT_MAP.json" ]; then
  # PORT_MAP is intentionally kept outside the upstream archive. Compare each
  # mapped source before showing the full diff so a sync can never look like a
  # routine version bump when a ported helper changed underneath us.
  while IFS= read -r upstream_path; do
    [ -n "$upstream_path" ] || continue
    old_path="$DEST/$upstream_path"
    new_path="$WORK_DIR/snapshot/$upstream_path"
    if [ -e "$old_path" ] || [ -e "$new_path" ]; then
      if [ ! -e "$old_path" ] || [ ! -e "$new_path" ] || ! cmp -s "$old_path" "$new_path"; then
        printf 'MANUAL REVIEW REQUIRED: mapped upstream source changed: %s\n' "$upstream_path" >&2
        MAPPED_CHANGED=1
      fi
    fi
  done < <(sed -n 's/.*"upstream": "\([^"]*\)".*/\1/p' "$DEST/PORT_MAP.json")
  if [ "$MAPPED_CHANGED" -eq 1 ]; then
    printf 'Mapped-source changes are not applied to RongVualt adapters; review PORT_MAP.json manually before merging.\n' >&2
  fi
fi
if [ -d "$DEST" ]; then
  diff -ruN \
    --exclude=SOURCE_MANIFEST.sha256 --exclude=UPSTREAM.md --exclude=PORT_MAP.json \
    "$DEST" "$WORK_DIR/snapshot" || true
fi
if [ "$APPLY" -eq 1 ]; then
  # Keep the old mirror recoverable and carry only RongVualt's port map into
  # the freshly archived upstream tree. Regenerate the source note so the
  # resolved immutable commit can never silently drift from the mirror.
  OLD_METADATA="$WORK_DIR/metadata"
  mkdir "$OLD_METADATA"
  if [ -f "$DEST/PORT_MAP.json" ]; then
    cp "$DEST/PORT_MAP.json" "$OLD_METADATA/PORT_MAP.json"
  fi
  NEW_DEST="$WORK_DIR/zipxtract.new"
  mv "$WORK_DIR/snapshot" "$NEW_DEST"
  if [ -f "$OLD_METADATA/PORT_MAP.json" ]; then
    cp "$OLD_METADATA/PORT_MAP.json" "$NEW_DEST/PORT_MAP.json"
  fi
  cat > "$NEW_DEST/UPSTREAM.md" <<EOF
# ZipXtract upstream snapshot

This directory is an immutable mirror of ZipXtract $REF at commit
$COMMIT.

- Repository: <https://github.com/WirelessAlien/ZipXtract>
- Snapshot date: $(date -u +%Y-%m-%d)
- License: GNU GPLv3 (see License)
- Local modifications: none. RongVualt code lives in zipxtract-core/ and
  materialfiles/.../archive/zipxtract/ instead of changing this mirror.

Use this script to fetch a new tag or immutable commit. It keeps the port map
and reports mapped-source changes for manual review.
EOF
  (
    cd "$NEW_DEST"
    find . -type f \
      ! -name SOURCE_MANIFEST.sha256 ! -name UPSTREAM.md ! -name PORT_MAP.json \
      -printf '%P\0' | sort -z | xargs -0 sha256sum
  ) > "$NEW_DEST/SOURCE_MANIFEST.sha256"
  BACKUP="$DEST.previous.$COMMIT"
  if [ -e "$BACKUP" ]; then
    echo "backup already exists: $BACKUP" >&2
    exit 3
  fi
  mv "$DEST" "$BACKUP"
  mv "$NEW_DEST" "$DEST"
  printf 'Applied mirror; previous snapshot kept at %s\n' "$BACKUP"
else
  printf '\nDiff shown above; the temporary snapshot is removed on exit.\n'
  printf 'Review it, then rerun with --apply to replace the mirror.\n'
fi
