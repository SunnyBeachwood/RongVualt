# Material Files UI migration boundary

RongVault's embedded file manager is a provider-only adaptation of the
Material Files workflow. Its normal window is launched through
`materialfiles/.../FileListActivity.kt` and uses the existing
`UnlockedDocumentsProvider`. The 1.2.0 integration restores
the upstream RootablePath/libsu service and the FTP page/service, but it still
does not import Material Files' network storage providers, APK installer, or
credential storage.

Material Files is distributed by its upstream project under GPL-3.0-or-later.
The EDS/RongVault project is also GPL-3.0-or-later, so a future import of
upstream source must retain each copyright header, the upstream license text,
and complete corresponding source/build instructions in the distributed
source archive. The current adapter is new RongVault code and does not claim
to be an unmodified Material Files release.

The embedded surface supports operations advertised by the EDS provider:
browsing, opening through a user-selected content URI, create, rename, delete,
file copy/move, and Root browsing of mounted filesystem paths. Root strategy
remains `NEVER`/`AUTOMATIC`/`ALWAYS` with `AUTOMATIC` as the default; denied,
timed-out, or revoked Root requests are surfaced as errors and block-device
writes/remounting are not exposed. The FTP service shares one immutable
ordinary, Root, or live-unlocked directory snapshot, rejects remote/archive
roots and path/symlink escapes, and runs as a user-enabled special-use
foreground service. Long-running copies and FTP activity use provider file
descriptors; no host plaintext temporary path is used. An unlocked FTP root is
process-only and is invalidated before its native session closes.

The imported Root and FTP sources retain their upstream GPL copyright headers.
libsu is linked under Apache-2.0 and Apache FtpServer/MINA under Apache-2.0;
the exact notices are recorded in `materialfiles/src/main/res/raw/licenses.xml`.

Normal file-manager windows now use the native Material 3 dual-pane shell in
`materialfiles/.../DualPaneFileListFragment.kt`. The shell keeps independent
paths, history, sorting, view mode, scroll state and selection for each pane;
AUTO switches to a 50:50 split at 600dp, while document/file pickers remain on
the legacy single-pane contract. Cross-pane copy/move is submitted to the same
foreground `FileJobService` used by archive extraction, and the right-pane
startup directory is configurable from Settings without opening remote or
encrypted locations automatically.

Before release, maintainers must re-check the exact upstream Material Files
revision, transitive dependency licenses, and any imported source files, then
ship the GPL notices and corresponding source for the exact APK build.
