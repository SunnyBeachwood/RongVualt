# RongVault user guide

## Open a container

Add an existing container or create one, choose **Unlock**, then enter its password. RongVault automatically checks outer and hidden volume headers. When the credentials cannot be verified, review the password, PIM and keyfiles.

## Security

PIM (0–2147468), cipher, KDF and keyfiles must match the container. Choose
among the six supported KDF hints; Argon2id shows its PIM-derived memory and
iteration estimate. Hidden-volume protection is for writing an outer volume
that contains a hidden volume. Its KDF remains auto-detected so native request
v2 and saved credential layouts stay compatible. Lock volumes when finished and
keep independent header backups. Forgotten passwords cannot be recovered.

## Root file access

In the built-in file manager settings choose `NEVER`, `AUTOMATIC` (default), or
`ALWAYS`. With root granted, the `/` entry can browse and manage mounted
`/data`, `/system`, `/vendor`, `/product`, and other filesystem paths. A denied,
timed-out, or revoked root request is shown as an error. Read-only mounts, AVB,
and kernel restrictions remain effective; block devices and remounting are not
supported. Shizuku/Sui remains disabled.

## FTP server

The FTP page can share ordinary storage, a Root path, or one directory in a
currently unlocked volume. The server listens on all interfaces, defaults to
port 2121, account login, and read-only access, and runs until you stop it.
Anonymous login and anonymous writes are explicit opt-ins. FTP content and
credentials are sent without encryption, so use it only on a trusted network.
An unlocked-volume share is process-only and stops before that volume is locked;
ordinary and Root shares do not persist any runtime document token.

## Archive and format compatibility

RongVault 1.3.0 integrates ZipXtract into the built-in file manager. It can
browse and extract ZIP/JAR, 7z, RAR/RAR5, TAR and compressed streams, create
ZIP/7z/TAR archives, and update only unencrypted 7z archives on safe writable
providers. Archive extraction is streamed and rejects traversal names and
archive links. The source, fixtures and device compatibility pass remain
pending release-first build verification.

## VeraCrypt compatibility

RongVault 1.3.0 opens and auto-detects all 15 VeraCrypt non-system XTS suites
and exposes the nine Windows creation suites for normal and hidden volumes.
The source expansion is pending build and compatibility verification. FAT and
exFAT can be read and written; NTFS is read-only. The app is independent and
is not affiliated with VeraCrypt.
