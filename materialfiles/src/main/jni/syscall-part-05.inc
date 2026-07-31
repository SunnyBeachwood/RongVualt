        struct passwd *passwd = getpwuid(getpwentUid);
        ++getpwentUid;
        errno = 0;
        if (passwd) {
            return passwd;
        }
    }
    return NULL;
}

void endpwent() {
    setpwent();
}

#endif

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_endpwent(JNIEnv *env, jclass clazz) {
    TEMP_FAILURE_RETRY_V(endpwent());
    if (errno) {
        throwSyscallException(env, "endpwent");
    }
}

JNIEXPORT jint JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_errno(
        JNIEnv *env, jclass clazz) {
    return errno;
}

JNIEXPORT jint JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_fcntl_1int(
        JNIEnv *env, jclass clazz, jobject javaFd, jint javaCmd, jint javaArg) {
    int fd = getFdFromFileDescriptor(env, javaFd);
    int cmd = javaCmd;
    int arg = javaArg;
    int result = TEMP_FAILURE_RETRY(fcntl(fd, cmd, arg));
    if (errno) {
        throwSyscallException(env, "fcntl");
        return 0;
    }
    return result;
}

JNIEXPORT jint JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_fcntl_1void(
        JNIEnv *env, jclass clazz, jobject javaFd, jint javaCmd) {
    int fd = getFdFromFileDescriptor(env, javaFd);
    int cmd = javaCmd;
    int result = TEMP_FAILURE_RETRY(fcntl(fd, cmd));
    if (errno) {
        throwSyscallException(env, "fcntl");
        return 0;
    }
    return result;
}

static jobject newStructGroup(JNIEnv *env, const struct group *group) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructGroupClass(env), "<init>",
                                 "(Lme/zhanghai/android/files/provider/common/ByteString;"
                                 "Lme/zhanghai/android/files/provider/common/ByteString;I"
                                 "[Lme/zhanghai/android/files/provider/common/ByteString;)V");
    }
    jobject gr_name;
    if (group->gr_name) {
        gr_name = newByteStringFromString(env, group->gr_name);
        if (!gr_name) {
            return NULL;
        }
    } else {
        gr_name = NULL;
    }
    jobject gr_passwd;
    if (group->gr_passwd) {
        gr_passwd = newByteStringFromString(env, group->gr_passwd);
        if (!gr_passwd) {
            return NULL;
        }
    } else {
        gr_passwd = NULL;
    }
    jint gr_gid = (jint) group->gr_gid;
    jobjectArray gr_mem;
    if (group->gr_mem) {
        jsize gr_memLength = 0;
        for (char **gr_memIterator = group->gr_mem; *gr_memIterator; ++gr_memIterator) {
            ++gr_memLength;
        }
        gr_mem = (*env)->NewObjectArray(env, gr_memLength, getByteStringClass(env), NULL);
        if (!gr_mem) {
            return NULL;
        }
        jsize gr_memIndex = 0;
        for (char **gr_memIterator = group->gr_mem; *gr_memIterator; ++gr_memIterator,
                ++gr_memIndex) {
            jobject gr_memElement = newByteStringFromString(env, *gr_memIterator);
            if (!gr_memElement) {
                return NULL;
            }
            (*env)->SetObjectArrayElement(env, gr_mem, gr_memIndex, gr_memElement);
            (*env)->DeleteLocalRef(env, gr_memElement);
        }
    } else {
        gr_mem = NULL;
    }
    return (*env)->NewObject(env, getStructGroupClass(env), constructor, gr_name, gr_passwd, gr_gid,
                             gr_mem);
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_getgrent(JNIEnv *env, jclass clazz) {
    while (true) {
        // getgrent() in bionic is thread safe.
        struct group *group = TEMP_FAILURE_RETRY_N(getgrent());
        if (errno) {
            throwSyscallException(env, "getgrent");
            return NULL;
        }
