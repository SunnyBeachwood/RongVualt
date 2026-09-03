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

## Compatibility

RongVault 1.1.0 opens and auto-detects all 15 VeraCrypt non-system XTS suites
and exposes the nine Windows creation suites for normal and hidden volumes.
The source expansion is pending build and compatibility verification. FAT and
exFAT can be read and written; NTFS is read-only. The app is independent and
is not affiliated with VeraCrypt.
