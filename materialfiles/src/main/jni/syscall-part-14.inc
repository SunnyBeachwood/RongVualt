    char *path = mallocStringFromByteString(env, javaPath);
    struct statvfs64 statvfs = {};
    TEMP_FAILURE_RETRY(statvfs64(path, &statvfs));
    free(path);
    if (errno) {
        throwSyscallException(env, "statvfs64");
        return NULL;
    }
    return newStructStatVfs(env, &statvfs);
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_symlink(
        JNIEnv *env, jclass clazz, jobject javaTarget, jobject javaLinkPath) {
    char *target = mallocStringFromByteString(env, javaTarget);
    char *linkPath = mallocStringFromByteString(env, javaLinkPath);
    TEMP_FAILURE_RETRY(symlink(target, linkPath));
    free(target);
    free(linkPath);
    if (errno) {
        throwSyscallException(env, "symlink");
    }
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_utimens(
        JNIEnv *env, jclass clazz, jobject javaPath, jobjectArray javaTimes) {
    doUtimens(env, javaPath, javaTimes, false);
}
