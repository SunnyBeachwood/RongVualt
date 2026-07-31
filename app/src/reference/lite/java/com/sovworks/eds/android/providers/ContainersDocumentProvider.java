package com.sovworks.eds.android.providers;

import android.annotation.TargetApi;
import android.os.Build;

import com.sovworks.eds.android.BuildConfig;

@TargetApi(Build.VERSION_CODES.KITKAT)
public class ContainersDocumentProvider extends ContainersDocumentProviderBase
{
    public static final String AUTHORITY = BuildConfig.APPLICATION_ID + ".providers.documents.lite";
}
