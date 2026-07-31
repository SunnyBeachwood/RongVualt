package org.eds.veracrypt.ui

import android.net.Uri
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import org.eds.veracrypt.domain.VeraCryptRepository
import org.eds.veracrypt.catalog.ContainerCatalogEntry
import org.eds.veracrypt.domain.VolumeError
import org.eds.veracrypt.domain.VolumeCredentials
import org.eds.veracrypt.domain.VolumeOpenOptions
import org.eds.veracrypt.domain.VolumeProbe
import org.eds.veracrypt.domain.VolumeSession

class VolumeOpenViewModel(private val repository: VeraCryptRepository) : ViewModel() {
    private val mutableState = MutableStateFlow<VolumeOpenUiState>(VolumeOpenUiState.Idle)
    val state: StateFlow<VolumeOpenUiState> = mutableState.asStateFlow()
    private var session: VolumeSession? = null

    fun probe(uri: Uri) = viewModelScope.launch {
        mutableState.value = VolumeOpenUiState.Probing
        try {
            mutableState.value = VolumeOpenUiState.Probed(repository.probe(uri))
        } catch (error: VolumeError) {
            mutableState.value = VolumeOpenUiState.Failed(error)
        } catch (error: CancellationException) {
            throw error
        } catch (error: Throwable) {
            mutableState.value = VolumeOpenUiState.Failed(VolumeError.IoInterrupted(error))
        }
    }

    fun open(entry: ContainerCatalogEntry, options: VolumeOpenOptions, credentials: VolumeCredentials) = viewModelScope.launch {
        mutableState.value = VolumeOpenUiState.Opening
        try {
            val previous = session
            if (previous != null) repository.close(previous)
            session = repository.open(entry, options, credentials)
            mutableState.value = VolumeOpenUiState.Opened(checkNotNull(session))
        } catch (error: VolumeError) {
            mutableState.value = VolumeOpenUiState.Failed(error)
        } catch (error: CancellationException) {
            throw error
        } catch (error: Throwable) {
            mutableState.value = VolumeOpenUiState.Failed(VolumeError.IoInterrupted(error))
        } finally {
            credentials.close()
            options.hiddenVolumeProtection?.close()
        }
    }

    fun closeSession() {
        val closing = session
        session = null
        mutableState.value = VolumeOpenUiState.Idle
        if (closing != null) viewModelScope.launch { repository.close(closing) }
    }

    override fun onCleared() {
        // An unlocked catalog session is application-owned and can still be
        // serving DocumentsProvider clients after this screen disappears.
        session = null
    }
}

sealed interface VolumeOpenUiState {
    data object Idle : VolumeOpenUiState
    data object Probing : VolumeOpenUiState
    data class Probed(val probe: VolumeProbe) : VolumeOpenUiState
    data object Opening : VolumeOpenUiState
    data class Opened(val session: VolumeSession) : VolumeOpenUiState
    data class Failed(val error: VolumeError) : VolumeOpenUiState
}
