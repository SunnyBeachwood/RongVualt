# About RongVault

**Version {{VERSION}}**

RongVault is an offline Android application for creating, opening, and managing
VeraCrypt-compatible file containers. It is independent and is not affiliated
with or endorsed by VeraCrypt.

## Project README

RongVault 1.3.0 supports standard and hidden volumes, automatic volume-type detection,
password/PIM authentication, all 15 VeraCrypt non-system XTS suites for opening,
the nine Windows creation suites, six KDF hints, biometric unlocking for eligible
saved credentials, and browsing through the built-in Android file manager. Root
file access through libsu and the local FTP server are also included; Root, FTP,
unlocked-volume sharing, and anonymous-write paths remain pending build and
compatibility verification. The application defaults to port 2121, account
login, and read-only FTP access, and warns that plain FTP is not encrypted.
FAT and exFAT volumes support read/write operations; NTFS is read-only. The
application does not decrypt a complete container onto plaintext storage.

The built-in file manager also integrates ZipXtract for streaming ZIP/JAR, 7z,
RAR/RAR5, TAR and compressed-stream extraction, ZIP/7z/TAR creation, and safe
updates of unencrypted 7z files. Archive paths remain read-only; traversal
names, archive links and unsafe provider replacements are rejected. The 1.3.0
archive surface is code-complete and awaits the release build and fixture/device
verification pass.

The complete project README, compatibility notes, build instructions, and
security limitations are available in the source repository:

[Read the full README on GitHub](https://github.com/SunnyBeachwood/RongVualt/blob/main/README.md)

[Open the RongVualt GitHub repository](https://github.com/SunnyBeachwood/RongVualt)

## License

RongVault is released under **GPL-3.0-or-later**. The complete GPL text is
included with the distribution.

## Open-source components

This release includes EDS Lite source heritage, Material Files 1.7.4, ZipXtract
v7.1.1, libsu 5.2.2,
Apache FtpServer 1.2.1/MINA 2.2.4, Botan,
FatFs, libyal NTFS libraries, Markwon, AndroidX, Kotlin and their runtime
dependencies. Their copyright and license notices are retained in the source
distribution and packaged notices.
