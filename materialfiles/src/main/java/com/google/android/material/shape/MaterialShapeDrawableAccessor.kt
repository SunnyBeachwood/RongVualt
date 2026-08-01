/*
 * Copyright (c) 2021 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package com.google.android.material.shape

import android.annotation.SuppressLint
import com.google.android.material.elevation.ElevationOverlayProvider

@Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")
class MaterialShapeDrawableAccessor private constructor() {
    companion object {
        @JvmStatic
        @SuppressLint("RestrictedApi")
        fun getElevationOverlayProvider(drawable: MaterialShapeDrawable): ElevationOverlayProvider {
            return (drawable.constantState as MaterialShapeDrawable.MaterialShapeDrawableState)
                .elevationOverlayProvider!!
        }

        @JvmStatic
        @SuppressLint("RestrictedApi")
        fun setElevationOverlayProvider(
            drawable: MaterialShapeDrawable,
            elevationOverlayProvider: ElevationOverlayProvider?
        ) {
            (drawable.constantState as MaterialShapeDrawable.MaterialShapeDrawableState)
                .elevationOverlayProvider = elevationOverlayProvider
        }

        @JvmStatic
        fun updateZ(drawable: MaterialShapeDrawable) {
            val parentAbsoluteElevation = drawable.parentAbsoluteElevation
            drawable.setParentAbsoluteElevation(parentAbsoluteElevation + 1)
            drawable.setParentAbsoluteElevation(parentAbsoluteElevation)
        }
    }
}
