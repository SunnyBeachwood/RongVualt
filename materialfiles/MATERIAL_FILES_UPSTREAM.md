# Material Files upstream provenance

This directory vendors the Material Files 1.7.4 source snapshot from
`C:/Projects/MaterialFiles` under GPL-3.0-or-later. Copyright headers are
retained in imported files. Firebase/Crashlytics and Shizuku/Sui remain
disabled. RongVault 1.2.0 restores the libsu RootService/RootablePath path and
the Apache FTP server with an application-owned bounded filesystem view; the
application manifest controls the exposed surface. Root and FTP additions are
source-only and remain pending build/verification for this release.

The restored RootService uses libsu 5.2.2 under Apache-2.0. FTP uses Apache
FtpServer 1.2.1 with Apache MINA 2.2.4 under Apache-2.0. See the in-app notice
file for attribution and the corresponding upstream source links.
