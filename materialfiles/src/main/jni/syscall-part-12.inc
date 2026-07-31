        return 0;
    }
    char *source = javaSource ? mallocStringFromByteString(env, javaSource) : NULL;
    char *target = mallocStringFromByteString(env, javaTarget);
    char *fileSystemType = javaFileSystemType ? mallocStringFromByteString(env, javaFileSystemType)
            : NULL;
    unsigned long mountFlags = (unsigned long) javaMountFlags;
    void *data = javaData ? (*env)->GetByteArrayElements(env, javaData, NULL) : NULL;
    int result = TEMP_FAILURE_RETRY(mount(source, target, fileSystemType, mountFlags, data));
    if (javaSource) {
        free(source);
    }
    free(target);
    if (javaFileSystemType) {
        free(fileSystemType);
    }
    if (javaData) {
        (*env)->ReleaseByteArrayElements(env, javaData, data, JNI_ABORT);
    }
    if (errno) {
        throwSyscallException(env, "mount");
        return 0;
    }
    return result;
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_open(
        JNIEnv *env, jclass clazz, jobject javaPath, jint javaFlags, jint javaMode) {
    char *path = mallocStringFromByteString(env, javaPath);
    int flags = javaFlags;
    mode_t mode = (mode_t) javaMode;
    int fd = TEMP_FAILURE_RETRY(open(path, flags, mode));
    free(path);
    if (errno) {
        throwSyscallException(env, "open");
        return NULL;
    }
    return newFileDescriptor(env, fd);
}

JNIEXPORT jlong JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_opendir(
        JNIEnv *env, jclass clazz, jobject javaPath) {
    char *path = mallocStringFromByteString(env, javaPath);
    DIR *dir = TEMP_FAILURE_RETRY_N(opendir(path));
    free(path);
    if (errno) {
        throwSyscallException(env, "opendir");
        return (jlong) NULL;
    }
    return (jlong) dir;
}

static jobject newStructDirent(JNIEnv *env, const struct dirent64 *dirent) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructDirentClass(env), "<init>",
                                 "(JJIILme/zhanghai/android/files/provider/common/ByteString;)V");
    }
    jlong d_ino = (jlong) dirent->d_ino;
    jlong d_off = dirent->d_off;
    jint d_reclen = dirent->d_reclen;
    jint d_type = dirent->d_type;
    jobject d_name = newByteStringFromString(env, dirent->d_name);
    if (!d_name) {
        return NULL;
    }
    return (*env)->NewObject(env, getStructDirentClass(env), constructor, d_ino, d_off, d_reclen,
                             d_type, d_name);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_readdir(
        JNIEnv *env, jclass clazz, jlong javaDir) {
    DIR *dir = (DIR *) javaDir;
    struct dirent64 *dirent = TEMP_FAILURE_RETRY_N(readdir64(dir));
    if (errno) {
        throwSyscallException(env, "readdir64");
        return NULL;
    }
    if (!dirent) {
        return NULL;
    }
    return newStructDirent(env, dirent);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_readlink(
        JNIEnv *env, jclass clazz, jobject javaPath) {
    char *path = mallocStringFromByteString(env, javaPath);
    size_t maxSize = PATH_MAX;
    jobject javaTarget = NULL;
    while (true) {
        //char target[maxSize] = {};
        char target[maxSize];
        size_t size = (size_t) TEMP_FAILURE_RETRY(readlink(path, target, maxSize));
        if (errno) {
            break;
        }
        if (size >= maxSize) {
            maxSize *= 2;
            continue;
        }
        javaTarget = newByteString(env, target, size);
        break;
    }
    free(path);
    if (errno) {
        throwSyscallException(env, "readlink");
        return NULL;
    }
    return javaTarget;
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_realpath(
        JNIEnv *env, jclass clazz, jobject javaPath) {
    char *path = mallocStringFromByteString(env, javaPath);
    char resolvedPath[PATH_MAX] = {};
