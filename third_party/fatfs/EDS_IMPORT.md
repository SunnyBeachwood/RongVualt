# FatFs Import

- Module: FatFs R0.13b
- Upstream author: ChaN
- Mirror: `https://github.com/pabigot/FatFs.git`
- Pinned mirror tag: `ff13b`
- Pinned mirror commit: `db06e15a75e39a6aa78fb606cfa8102ed1389520`
- Imported source hash (unmodified `ff.c`):
  `125b0c70030f03fff85013c82c6810e455f3f7903063083d0677d13b025b9427`
- Imported source hash (unmodified `ffunicode.c`):
  `37b594b653b0e5c6fbf25428ccb56240e5c8149d786a22d0b74f17d8fd936341`
- License: FatFs permissive one-condition license, retained in the headers of
  `ff.c` and `ff.h`.

The only local source modification is `ffconf.h`. It enables one read/write
volume, UTF-8 long filenames, exFAT, `f_mkfs`, and variable 512-4096 byte
logical sectors. The original module source is retained verbatim otherwise.
