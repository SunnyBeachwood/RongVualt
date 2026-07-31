
static jobject newStructMntent(JNIEnv *env, const struct mntent *mntent) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructMntentClass(env), "<init>",
                                 "(Lme/zhanghai/android/files/provider/common/ByteString;"
                                 "Lme/zhanghai/android/files/provider/common/ByteString;"
                                 "Lme/zhanghai/android/files/provider/common/ByteString;"
                                 "Lme/zhanghai/android/files/provider/common/ByteString;II)V");
    }
    jobject mnt_fsname = newByteStringFromString(env, mntent->mnt_fsname);
    if (!mnt_fsname) {
        return NULL;
    }
    jobject mnt_dir = newByteStringFromString(env, mntent->mnt_dir);
    if (!mnt_dir) {
        return NULL;
    }
    jobject mnt_type = newByteStringFromString(env, mntent->mnt_type);
    if (!mnt_type) {
        return NULL;
    }
    jobject mnt_opts = newByteStringFromString(env, mntent->mnt_opts);
    if (!mnt_opts) {
        return NULL;
    }
    jint mnt_freq = mntent->mnt_freq;
    jint mnt_passno = mntent->mnt_passno;
    return (*env)->NewObject(env, getStructMntentClass(env), constructor, mnt_fsname, mnt_dir,
            mnt_type, mnt_opts, mnt_freq, mnt_passno);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_getmntent(
        JNIEnv *env, jclass clazz, jlong javaFile) {
    FILE *file = (FILE *) javaFile;
#if __ANDROID_API__ >= __ANDROID_API_L_MR1__
    // getmntent() in bionic is thread safe.
    struct mntent *mntent = TEMP_FAILURE_RETRY_N(getmntent(file));
#else
    // getmntent() in bionic is a stub until API 22.
    struct mntent entryBuffer = {};
    char stringsBuffer[BUFSIZ] = {};
    struct mntent *mntent = TEMP_FAILURE_RETRY_N(_getmntent_r(file, &entryBuffer, stringsBuffer,
            sizeof(stringsBuffer)));
#endif
    if (errno) {
        throwSyscallException(env, "getmntent");
        return NULL;
    }
    if (!mntent) {
        return NULL;
    }
    return newStructMntent(env, mntent);
}

static jobject newStructPasswd(JNIEnv *env, const struct passwd *passwd) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructPasswdClass(env), "<init>",
                "(Lme/zhanghai/android/files/provider/common/ByteString;II"
                "Lme/zhanghai/android/files/provider/common/ByteString;"
                "Lme/zhanghai/android/files/provider/common/ByteString;"
                "Lme/zhanghai/android/files/provider/common/ByteString;)V");
    }
    jobject pw_name;
    if (passwd->pw_name) {
        pw_name = newByteStringFromString(env, passwd->pw_name);
        if (!pw_name) {
            return NULL;
        }
    } else {
        pw_name = NULL;
    }
    jint pw_uid = (jint) passwd->pw_uid;
    jint pw_gid = (jint) passwd->pw_gid;
#ifdef __LP64__
    jobject pw_gecos;
    if (passwd->pw_gecos) {
        pw_gecos = newByteStringFromString(env, passwd->pw_gecos);
        if (!pw_gecos) {
            return NULL;
        }
    } else {
        pw_gecos = NULL;
    }
#else
    jobject pw_gecos = NULL;
#endif
    jobject pw_dir;
    if (passwd->pw_dir) {
        pw_dir = newByteStringFromString(env, passwd->pw_dir);
        if (!pw_dir) {
            return NULL;
        }
    } else {
        pw_dir = NULL;
    }
    jobject pw_shell;
    if (passwd->pw_shell) {
        pw_shell = newByteStringFromString(env, passwd->pw_shell);
        if (!pw_shell) {
            return NULL;
        }
    } else {
        pw_shell = NULL;
    }
    return (*env)->NewObject(env, getStructPasswdClass(env), constructor, pw_name, pw_uid, pw_gid,
                             pw_gecos, pw_dir, pw_shell);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_getpwent(JNIEnv *env, jclass clazz) {
    while (true) {
        // getpwent() in bionic is thread safe.
        struct passwd *passwd = TEMP_FAILURE_RETRY_N(getpwent());
        if (errno) {
            throwSyscallException(env, "getpwent");
            return NULL;
        }
