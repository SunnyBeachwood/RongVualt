/*
 * Copyright (c) 2024 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

@file:Suppress("INVISIBLE_MEMBER", "INVISIBLE_REFERENCE")

package at.bitfire.dav4jvm

import at.bitfire.dav4jvm.exception.DavException
import at.bitfire.dav4jvm.exception.HttpException
import java.io.IOException
import okhttp3.Response

class DavResourceAccessor private constructor() {
    companion object {
        @JvmStatic
        @Throws(HttpException::class)
        fun checkStatus(davResource: DavResource, response: Response) {
            davResource.checkStatus(response)
        }

        @JvmStatic
        @Throws(DavException::class, IOException::class)
        fun followRedirects(davResource: DavResource, sendRequest: () -> Response): Response =
            davResource.followRedirects(sendRequest)
    }
}
