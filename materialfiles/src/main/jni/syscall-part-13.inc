    TEMP_FAILURE_RETRY_N(realpath(path, resolvedPath));
    free(path);
    if (errno) {
        throwSyscallException(env, "realpath");
        return NULL;
    }
    return newByteStringFromString(env, resolvedPath);
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_remove(
        JNIEnv *env, jclass clazz, jobject javaPath) {
    char *path = mallocStringFromByteString(env, javaPath);
    int result = TEMP_FAILURE_RETRY(remove(path));
    free(path);
    // This is a libc function and doesn't clear errno properly.
    //if (errno) {
    if (result) {
        throwSyscallException(env, "remove");
    }
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_rename(
        JNIEnv *env, jclass clazz, jobject javaOldPath, jobject javaNewPath) {
    char *oldPath = mallocStringFromByteString(env, javaOldPath);
    char *newPath = mallocStringFromByteString(env, javaNewPath);
    TEMP_FAILURE_RETRY(rename(oldPath, newPath));
    free(oldPath);
    free(newPath);
    if (errno) {
        throwSyscallException(env, "rename");
    }
}

JNIEXPORT jlong JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_sendfile(
        JNIEnv* env, jclass clazz, jobject javaOutFd, jobject javaInFd, jobject javaOffset,
        jlong javaCount) {
    int outFd = getFdFromFileDescriptor(env, javaOutFd);
    int inFd = getFdFromFileDescriptor(env, javaInFd);
    off64_t offset = 0;
    off64_t* offsetPointer = NULL;
    if (javaOffset) {
        offset = (*env)->GetLongField(env, javaOffset, getInt64RefValueField(env));
        offsetPointer = &offset;
    }
    size_t count = (size_t) javaCount;
    long result = TEMP_FAILURE_RETRY(sendfile64(outFd, inFd, offsetPointer, count));
    if (errno) {
        throwSyscallException(env, "sendfile64");
        return 0;
    }
    if (javaOffset) {
        (*env)->SetLongField(env, javaOffset, getInt64RefValueField(env), offset);
    }
    return result;
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_setgrent(JNIEnv *env, jclass clazz) {
    TEMP_FAILURE_RETRY_V(setgrent());
    if (errno) {
        throwSyscallException(env, "setgrent");
    }
}

JNIEXPORT jlong JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_setmntent(
        JNIEnv *env, jclass clazz, jobject javaPath, jobject javaMode) {
    char *path = mallocStringFromByteString(env, javaPath);
    char *mode = mallocStringFromByteString(env, javaMode);
    FILE *file = TEMP_FAILURE_RETRY_N(setmntent(path, mode));
    free(path);
    free(mode);
    if (errno) {
        throwSyscallException(env, "setmntent");
        return (jlong) NULL;
    }
    return (jlong) file;
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_setpwent(JNIEnv *env, jclass clazz) {
    TEMP_FAILURE_RETRY_V(setpwent());
    if (errno) {
        throwSyscallException(env, "setpwent");
    }
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_stat(
        JNIEnv *env, jclass clazz, jobject javaPath) {
    return doStat(env, javaPath, false);
}

static jobject newStructStatVfs(JNIEnv *env, const struct statvfs64 *statvfs) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructStatVfsClass(env), "<init>", "(JJJJJJJJJJJ)V");
    }
    jlong f_bsize = statvfs->f_bsize;
    jlong f_frsize = statvfs->f_frsize;
    jlong f_blocks = statvfs->f_blocks;
    jlong f_bfree = statvfs->f_bfree;
    jlong f_bavail = statvfs->f_bavail;
    jlong f_files = statvfs->f_files;
    jlong f_ffree = statvfs->f_ffree;
    jlong f_favail = statvfs->f_favail;
    jlong f_fsid = statvfs->f_fsid;
    jlong f_flag = statvfs->f_flag;
    jlong f_namemax = statvfs->f_namemax;
    return (*env)->NewObject(env, getStructStatVfsClass(env), constructor, f_bsize, f_frsize,
                             f_blocks, f_bfree, f_bavail, f_files, f_ffree, f_favail, f_fsid,
                             f_flag, f_namemax);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_statvfs(
        JNIEnv *env, jclass clazz, jobject javaPath) {
