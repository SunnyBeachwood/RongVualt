package org.eds.veracrypt.ui

import android.os.Bundle
import android.view.WindowManager
import androidx.fragment.app.Fragment

/** Base class for password, PIM, keyfile and credential-recovery screens. */
abstract class SensitiveFragment : Fragment() {
    override fun onResume() {
        super.onResume()
        requireActivity().window.addFlags(WindowManager.LayoutParams.FLAG_SECURE)
    }

    override fun onPause() {
        requireActivity().window.clearFlags(WindowManager.LayoutParams.FLAG_SECURE)
        super.onPause()
    }
}
