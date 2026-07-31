package org.eds.veracrypt.ui

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import io.noties.markwon.Markwon
import com.sovworks.eds.android.R
import com.sovworks.eds.android.BuildConfig
import com.sovworks.eds.android.databinding.FragmentInfoBinding

/** Offline help/about content; no network access is needed. */
class InfoFragment : Fragment() {
    private var binding: FragmentInfoBinding? = null

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, state: Bundle?): View =
        FragmentInfoBinding.inflate(inflater, container, false).also { binding = it }.root

    override fun onViewCreated(view: View, state: Bundle?) {
        val about = requireArguments().getBoolean(ARG_ABOUT)
        val raw = resources.openRawResource(if (about) R.raw.rongvault_readme else R.raw.rongvault_help)
            .bufferedReader(Charsets.UTF_8).use { it.readText() }
            .replace("{{VERSION}}", BuildConfig.VERSION_NAME)
            .let { markdown ->
                if (about) {
                    "$markdown\n\n---\n\n[Open the RongVualt GitHub repository](https://github.com/SunnyBeachwood/RongVualt)"
                } else {
                    markdown
                }
            }
        Markwon.builder(requireContext()).build().setMarkdown(binding!!.infoBody, raw)
        (activity as? ContainerCatalogActivity)?.setInfoTitle(
            if (about) R.string.rv_menu_about else R.string.rv_menu_help,
        )
    }

    override fun onDestroyView() {
        binding = null
        super.onDestroyView()
    }

    companion object {
        private const val ARG_ABOUT = "about"

        fun newInstance(about: Boolean) = InfoFragment().apply {
            arguments = Bundle().apply { putBoolean(ARG_ABOUT, about) }
        }
    }
}
