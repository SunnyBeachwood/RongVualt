package com.sovworks.eds.android.filemanager.fragments;

import android.os.Bundle;
import androidx.annotation.Nullable;
import com.trello.rxlifecycle2.android.FragmentEvent;
import com.trello.rxlifecycle2.components.RxActivity;
import com.trello.rxlifecycle2.components.RxFragment;

import io.reactivex.Completable;
import io.reactivex.subjects.CompletableSubject;


public class ExtStorageWritePermisisonCheckFragment extends RxFragment
{
    public static final String TAG = "com.sovworks.eds.android.filemanager.fragments.ExtStorageWritePermisisonCheckFragment";

    public static Completable getObservable(RxActivity activity)
    {
        // minSdk 29 uses SAF grants for external storage. Broad storage
        // permissions are intentionally absent from the modern manifest.
        return Completable.complete();
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        lifecycle().
                filter(event -> event == FragmentEvent.RESUME).
                firstElement().
                subscribe((event) -> _extStoragePermissionCheckSubject.onComplete());
    }

    public void cancelExtStoragePermissionRequest()
    {
        _extStoragePermissionCheckSubject.onComplete();
        getFragmentManager().beginTransaction().remove(this).commitAllowingStateLoss();
    }

    /**
     * Kept for legacy callers. Android 10+ uses persisted SAF grants instead
     * of the removed broad external-storage write permission.
     */
    public void requestExtStoragePermission()
    {
        _extStoragePermissionCheckSubject.onComplete();
    }

    private final CompletableSubject _extStoragePermissionCheckSubject = CompletableSubject.create();
}
