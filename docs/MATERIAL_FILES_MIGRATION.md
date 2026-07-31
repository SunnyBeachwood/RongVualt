# Material Files UI migration boundary

RongVault's embedded file manager is a provider-only adaptation of the
Material Files workflow. It is implemented in
`app/src/main/kotlin/org/eds/veracrypt/ui/EmbeddedFileBrowserActivity.kt` and
uses the existing `UnlockedDocumentsProvider`; it does not import Material
Files' application manifest, network providers, root/Shizuku integration,
FTP server, APK installer, or credential storage.

Material Files is distributed by its upstream project under GPL-3.0-or-later.
The EDS/RongVault project is also GPL-3.0-or-later, so a future import of
upstream source must retain each copyright header, the upstream license text,
and complete corresponding source/build instructions in the distributed
source archive. The current adapter is new RongVault code and does not claim
to be an unmodified Material Files release.

The embedded surface intentionally supports only operations advertised by the
EDS provider: browsing, opening through a user-selected content URI, create,
rename, delete, and file copy/move within the current unlocked volume. All
long-running copies run under the volume foreground-operation guard and use
provider file descriptors; no host plaintext temporary path is used.

Before release, maintainers must re-check the exact upstream Material Files
revision, transitive dependency licenses, and any imported source files, then
ship the GPL notices and corresponding source for the exact APK build.
