# About RongVault

**Version {{VERSION}}**

RongVault is an offline Android application for creating, opening, and managing
VeraCrypt-compatible file containers. It is independent and is not affiliated
with or endorsed by VeraCrypt.

## Project README

RongVault supports standard and hidden volumes, automatic volume-type detection,
password/PIM authentication, all 15 VeraCrypt non-system XTS suites for opening,
the nine Windows creation suites, six KDF hints, biometric unlocking for eligible
saved credentials, and browsing through the built-in Android file manager. The
1.1.0 algorithm/KDF expansion is pending build and compatibility verification.
FAT and exFAT volumes support read/write operations; NTFS is read-only. The
application does not decrypt a complete container onto plaintext storage.

The complete project README, compatibility notes, build instructions, and
security limitations are available in the source repository:

[Read the full README on GitHub](https://github.com/SunnyBeachwood/RongVualt/blob/main/README.md)

[Open the RongVualt GitHub repository](https://github.com/SunnyBeachwood/RongVualt)

## License

RongVault is released under **GPL-3.0-or-later**. The complete GPL text is
included with the distribution.

## Open-source components

This release includes EDS Lite source heritage, Material Files 1.7.4, Botan,
FatFs, libyal NTFS libraries, Markwon, AndroidX, Kotlin and their runtime
dependencies. Their copyright and license notices are retained in the source
distribution and packaged notices.
