/*
 * Copyright (C) 2026 RongVualt contributors
 * Licensed under the GNU GPL v3.
 */

package org.eds.zipxtract.core

/**
 * Central policy used by UI and jobs before exposing the 7z mutation entry
 * points. Password-bearing archives and providers without an atomic sibling
 * replacement are intentionally browse/extract-only.
 */
object ArchiveEditPolicy {
    fun canUpdate7z(
        probe: ArchiveProbe,
        providerSupportsReplacement: Boolean,
        password: CharArray? = null,
    ): Boolean = probe.format == ArchiveFormat.SEVEN_ZIP &&
        probe.encryption == ArchiveEncryption.NONE &&
        providerSupportsReplacement &&
        (password?.isEmpty() != false)
}
