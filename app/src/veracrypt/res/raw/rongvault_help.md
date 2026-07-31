# RongVault user guide

## Open a container

Add an existing container or create one, choose **Unlock**, then enter its password. RongVault automatically checks outer and hidden volume headers. When the credentials cannot be verified, review the password, PIM and keyfiles.

## Security

PIM, cipher, KDF and keyfiles must match the container. Hidden-volume protection is for writing an outer volume that contains a hidden volume. Lock volumes when finished and keep independent header backups. Forgotten passwords cannot be recovered.

## Compatibility

RongVault opens VeraCrypt-compatible non-system file containers. FAT and exFAT can be read and written; NTFS is read-only. The app is independent and is not affiliated with VeraCrypt.
