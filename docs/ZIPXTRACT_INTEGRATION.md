# RongVualt 1.3.1 archive integration

RongVualt embeds ZipXtract's archive engines behind a small provider-neutral
`zipxtract-core` library. Material Files supplies the `java8.nio.file.Path`
adapter, so local storage, Root paths and unlocked DocumentsProvider volumes use
the same extraction, creation and 7z update jobs. The archive file system stays
read-only; only unencrypted 7z files in a safe writable provider can be replaced
through the backup/verify/rollback job.

The file manager exposes three extraction destinations (here, a sibling folder,
or a selected folder), a compact archive dialog with an advanced options area,
and an exported `ArchiveIntentActivity` for explicit archive `VIEW` and file
`SEND`/`SEND_MULTIPLE` intents. URI grants are forwarded only for `content://`
and `file://` sources; `FileListActivity` remains non-exported.

## Split and encryption behavior

ZIP creation uses Zip4j's standard `.z01… + .zip` volumes. 7z creation emits
standard `.7z.001`, `.7z.002`, … volumes through a private sequential staging
writer. Both formats require at least 64 KiB per volume and ask for confirmation
before creating more than 100 volumes. Extraction resolves the complete set when
any volume is selected and reports the first missing volume before creating the
destination directory.

ZIP offers AES-256 (the UI default), AES-128, and legacy ZIP encryption. 7z
content encryption is AES-256; its header encryption, which hides file names,
is an independent optional switch. ZIP encryption never hides file names.

## Upstream boundary

`third_party/zipxtract/` is a read-only v7.1.1 snapshot. Do not edit it to fix
RongVualt behavior. The port map records the source-to-core correspondence;
`tools/sync-zipxtract-upstream.sh [--apply] <ref>` stages a new immutable ref,
prints a diff and, with `--apply`, replaces only the mirror while preserving the
port metadata. Any changed mapped source requires manual review.

## Verification status

Version 1.3.1 is **代码完成，未编译/未验证**. This turn intentionally adds code
and tests only. The first authorized verification pass must build the signed
release arm64 artifact (`RongVualt-1.3.0-arm64_v8.apk`) and then run unit,
instrumentation and device compatibility tests in one batch.
