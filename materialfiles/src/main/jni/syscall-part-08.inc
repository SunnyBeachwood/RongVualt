        if (!passwd) {
            return NULL;
        }
        if (passwd->pw_name[0] == 'o' && passwd->pw_name[1] == 'e' && passwd->pw_name[2] == 'm'
            && passwd->pw_name[3] == '_') {
            continue;
        }
        if (passwd->pw_name[0] == 'u' && passwd->pw_name[1] >= '0' && passwd->pw_name[1] <= '9') {
            return NULL;
        }
        return newStructPasswd(env, passwd);
    }
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_getpwnam(
        JNIEnv *env, jclass clazz, jobject javaName) {
    char *name = mallocStringFromByteString(env, javaName);
    size_t bufferSize = (size_t) sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufferSize == -1) {
        // See `man 3 getpwnam`
        bufferSize = 16384;
    }
    //char buffer[bufferSize] = {};
    char buffer[bufferSize];
    struct passwd passwd = {};
    struct passwd *result = NULL;
    errno = TEMP_FAILURE_RETRY_E(getpwnam_r(name, &passwd, buffer, bufferSize, &result));
    free(name);
    if (errno) {
        throwSyscallException(env, "getpwnam_r");
        return NULL;
    }
    if (!result) {
        return NULL;
    }
    return newStructPasswd(env, result);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_getpwuid(
        JNIEnv *env, jclass clazz, jint javaUid) {
    uid_t uid = (uid_t) javaUid;
    size_t bufferSize = (size_t) sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufferSize == -1) {
        // See `man 3 getpwuid`
        bufferSize = 16384;
    }
    //char buffer[bufferSize] = {};
    char buffer[bufferSize];
    struct passwd passwd = {};
    struct passwd *result = NULL;
    errno = TEMP_FAILURE_RETRY_E(getpwuid_r(uid, &passwd, buffer, bufferSize, &result));
    if (errno) {
        throwSyscallException(env, "getpwnam_r");
        return NULL;
    }
    if (!result) {
        return NULL;
    }
    return newStructPasswd(env, result);
}

static char *mallocMntOptsFromStructMntent(JNIEnv *env, jobject javaMntent) {
    jobject javaOpts = (*env)->GetObjectField(env, javaMntent, getStructMntentMntOptsField(env));
    return mallocStringFromByteString(env, javaOpts);
}

#if __ANDROID_API__ < __ANDROID_API_O__
static char* _hasmntopt(const struct mntent* mnt, const char* opt) {
    char* token = mnt->mnt_opts;
    char* const end = mnt->mnt_opts + strlen(mnt->mnt_opts);
    const size_t optLen = strlen(opt);
    while (token) {
        char* const tokenEnd = token + optLen;
        if (tokenEnd > end) break;
        if (memcmp(token, opt, optLen) == 0 &&
            (*tokenEnd == '\0' || *tokenEnd == ',' || *tokenEnd == '=')) {
            return token;
        }
        token = strchr(token, ',');
        if (token) token++;
    }
    return NULL;
}
#endif

JNIEXPORT jboolean JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_hasmntopt(
        JNIEnv *env, jclass clazz, jobject javaMntent, jobject javaOption) {
    struct mntent mntent = {};
    mntent.mnt_opts = mallocMntOptsFromStructMntent(env, javaMntent);
    char *option = mallocStringFromByteString(env, javaOption);
#if __ANDROID_API__ >= __ANDROID_API_O__
    char *match = hasmntopt(&mntent, option);
#else
    char *match = _hasmntopt(&mntent, option);
#endif
    free(mntent.mnt_opts);
    free(option);
    bool hasOption = match != NULL;
    return (jboolean) hasOption;
}

JNIEXPORT jint JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_inotify_1add_1watch(
        JNIEnv *env, jclass clazz, jobject javaFd, jobject javaPath, jint javaMask) {
    int fd = getFdFromFileDescriptor(env, javaFd);
    char *path = mallocStringFromByteString(env, javaPath);
    uint32_t mask = (uint32_t) javaMask;
    int wd = TEMP_FAILURE_RETRY(inotify_add_watch(fd, path, mask));
    free(path);
    if (errno) {
        throwSyscallException(env, "inotify_add_watch");
        return 0;
    }
    return wd;
}

JNIEXPORT jobject JNICALL
