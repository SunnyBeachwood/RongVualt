/*
 * Copyright (c) 2020 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

package com.hierynomus.smbj.share

import com.hierynomus.mssmb2.SMB2FileId
import com.hierynomus.mssmb2.messages.SMB2IoctlResponse
import com.hierynomus.smbj.io.ArrayByteChunkProvider
import java.util.concurrent.Future

class ShareAccessor private constructor() {
    companion object {
        /**
         * This ioctl() variant allows passing in the statusHandler.
         *
         * @see Share.ioctl
         */
        @JvmStatic
        fun ioctl(
            share: Share,
            fileId: SMB2FileId,
            ctlCode: Long,
            isFsCtl: Boolean,
            inData: ByteArray,
            inOffset: Int,
            inLength: Int,
            statusHandler: StatusHandler,
            timeout: Long
        ): SMB2IoctlResponse {
            val inputData = ArrayByteChunkProvider(inData, inOffset, inLength, 0)
            val future: Future<SMB2IoctlResponse> =
                share.ioctlAsync(fileId, ctlCode, isFsCtl, inputData, -1)
            return share.receive(future, "IOCTL", fileId, statusHandler, timeout)
        }
    }
}
