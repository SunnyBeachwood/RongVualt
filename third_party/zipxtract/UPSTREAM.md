# ZipXtract upstream snapshot

This directory is an immutable, read-only mirror of ZipXtract `v7.1.1` at
commit `c137bc1e07e8bea89f13dc26a938fd35df62918c`.

- Repository: <https://github.com/WirelessAlien/ZipXtract>
- Snapshot date: 2026-09-05
- License: GNU GPLv3 (see `License`)
- Local modifications: none. RongVualt code lives in `zipxtract-core/` and
  `materialfiles/.../archive/zipxtract/` instead of changing this mirror.

Use `tools/sync-zipxtract-upstream.sh` to fetch a new tag or immutable commit.
The script stages a fresh checkout, verifies the resolved commit and writes a
new hash manifest; it never overwrites the RongVualt adapter or port map.
