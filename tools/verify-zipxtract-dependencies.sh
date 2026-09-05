#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
CORE="$ROOT_DIR/zipxtract-core/build.gradle"
APP="$ROOT_DIR/app/build.gradle"
PROGUARD="$ROOT_DIR/zipxtract-core/consumer-rules.pro"
META="$ROOT_DIR/third_party/zipxtract/UPSTREAM.md"
MANIFEST="$ROOT_DIR/third_party/zipxtract/SOURCE_MANIFEST.sha256"

grep -q 'c137bc1e07e8bea89f13dc26a938fd35df62918c' "$META"
grep -q "commons-compress:1.28.0" "$CORE"
grep -q "org.brotli:dec:0.1.2" "$CORE"
grep -q "xz:1.10" "$CORE"
grep -q "zip4j:2.11.5" "$CORE"
grep -q "7-Zip-JBinding-4Android:Release-16.02-2.03" "$CORE"
grep -q "libarchive:library:1.1.6" "$CORE"
grep -q "zstd-jni:1.5.7-6" "$CORE"
grep -q "abiFilters 'arm64-v8a'" "$APP"
grep -q 'net.sf.sevenzipjbinding' "$PROGUARD"
grep -q 'org.brotli.dec' "$PROGUARD"
grep -R -q 'GNU GPL v3' "$ROOT_DIR/zipxtract-core/src/main"
(cd "$(dirname "$MANIFEST")" && sha256sum -c "$(basename "$MANIFEST")" >/dev/null)

echo 'ZipXtract mirror, fixed dependencies, GPL headers, ProGuard and arm64 checks passed.'
