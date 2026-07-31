    free(name);
    (*env)->ReleaseByteArrayElements(env, javaValue, value, JNI_ABORT);
    if (errno) {
        throwSyscallException(env, "lsetxattr");
    }
}

static jobject newStructTimespec(JNIEnv *env, const struct timespec *timespec) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructTimespecClass(env), "<init>", "(JJ)V");
    }
    jlong tv_sec = timespec->tv_sec;
    jlong tv_nsec = timespec->tv_nsec;
    return (*env)->NewObject(env, getStructTimespecClass(env), constructor, tv_sec, tv_nsec);
}

static jobject newStructStat(JNIEnv *env, const struct stat64 *stat) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructStatClass(env), "<init>", "(JJIJIIJJJJ"
                "Lme/zhanghai/android/files/provider/linux/syscall/StructTimespec;"
                "Lme/zhanghai/android/files/provider/linux/syscall/StructTimespec;"
                "Lme/zhanghai/android/files/provider/linux/syscall/StructTimespec;)V");
    }
    jlong st_dev = (jlong) stat->st_dev;
    jlong st_ino = (jlong) stat->st_ino;
    jint st_mode = (jint) stat->st_mode;
    jlong st_nlink = stat->st_nlink;
    jint st_uid = (jint) stat->st_uid;
    jint st_gid = (jint) stat->st_gid;
    jlong st_rdev = (jlong) stat->st_rdev;
    jlong st_size = stat->st_size;
    jlong st_blksize = stat->st_blksize;
    jlong st_blocks = (jlong) stat->st_blocks;
    jobject st_atim = newStructTimespec(env, &stat->st_atim);
    if (!st_atim) {
        return NULL;
    }
    jobject st_mtim = newStructTimespec(env, &stat->st_mtim);
    if (!st_mtim) {
        return NULL;
    }
    jobject st_ctim = newStructTimespec(env, &stat->st_ctim);
    if (!st_ctim) {
        return NULL;
    }
    return (*env)->NewObject(env, getStructStatClass(env), constructor, st_dev, st_ino, st_mode,
                             st_nlink, st_uid, st_gid, st_rdev, st_size, st_blksize, st_blocks,
                             st_atim, st_mtim, st_ctim);
}

static jobject doStat(JNIEnv *env, jobject javaPath, bool isLstat) {
    char *path = mallocStringFromByteString(env, javaPath);
    struct stat64 stat = {};
    TEMP_FAILURE_RETRY((isLstat ? lstat64 : stat64)(path, &stat));
    free(path);
    if (errno) {
        throwSyscallException(env, isLstat ? "lstat64" : "stat64");
        return NULL;
    }
    return newStructStat(env, &stat);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_lstat(
        JNIEnv *env, jclass clazz, jobject javaPath) {
    return doStat(env, javaPath, true);
}

static void readStructTimespec(JNIEnv *env, jobject javaTime, struct timespec *time) {
    time->tv_sec = (time_t) (*env)->GetLongField(env, javaTime, getStructTimespecTvSecField(env));
    time->tv_nsec = (long) (*env)->GetLongField(env, javaTime, getStructTimespecTvNsecField(env));
}

JNIEXPORT void JNICALL
doUtimens(JNIEnv *env, jobject javaPath, jobjectArray javaTimes, bool isLutimens) {
    char *path = mallocStringFromByteString(env, javaPath);
    size_t timesSize = (size_t) (*env)->GetArrayLength(env, javaTimes);
    //struct timespec times[timesSize] = {};
    struct timespec times[timesSize];
    for (size_t i = 0; i < timesSize; ++i) {
        jsize javaTimeIndex = (jsize) i;
        jobject javaTime = (*env)->GetObjectArrayElement(env, javaTimes, javaTimeIndex);
        readStructTimespec(env, javaTime, &times[i]);
        (*env)->DeleteLocalRef(env, javaTime);
    }
    TEMP_FAILURE_RETRY(utimensat(AT_FDCWD, path, times, isLutimens ? AT_SYMLINK_NOFOLLOW : 0));
    free(path);
    if (errno) {
        throwSyscallException(env, "utimensat");
    }
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_lutimens(
        JNIEnv *env, jclass clazz, jobject javaPath, jobjectArray javaTimes) {
    doUtimens(env, javaPath, javaTimes, true);
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_mkdir(
        JNIEnv *env, jclass clazz, jobject javaPath, jint javaMode) {
    char *path = mallocStringFromByteString(env, javaPath);
    mode_t mode = (mode_t) javaMode;
    TEMP_FAILURE_RETRY(mkdir(path, mode));
    free(path);
    if (errno) {
        throwSyscallException(env, "mkdir");
    }
}

JNIEXPORT jint JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_mount(
        JNIEnv *env, jclass clazz, jobject javaSource, jobject javaTarget,
        jobject javaFileSystemType, jlong javaMountFlags, jbyteArray javaData) {
    if (geteuid() != 0) {
        // Avoid getting killed by seccomp.
        errno = EPERM;
        throwSyscallException(env, "mount");
