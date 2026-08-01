/*
 * Copyright (c) 2020 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package com.hierynomus.smbj.share

import com.hierynomus.mssmb2.messages.SMB2ReadResponse
import com.hierynomus.smbj.common.SMBRuntimeException
import java.util.concurrent.Future

class FileAccessor private constructor() {
    companion object {
        /** @see File.readAsync */
        @JvmStatic
        @Throws(SMBRuntimeException::class)
        fun readAsync(file: File, offset: Long, length: Int): Future<SMB2ReadResponse> =
            file.readAsync(offset, length)
    }
}
