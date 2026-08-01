/*
 * Copyright (c) 2021 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package net.schmizz.sshj.sftp

import net.schmizz.concurrent.Promise
import java.io.IOException

class RemoteFileAccessor private constructor() {
    companion object {
        @JvmStatic
        @Throws(IOException::class)
        fun asyncRead(file: RemoteFile, offset: Long, length: Int): Promise<Response, SFTPException> =
            file.asyncRead(offset, length)

        @JvmStatic
        fun getRequester(file: RemoteFile): SFTPEngine = file.requester
    }
}
