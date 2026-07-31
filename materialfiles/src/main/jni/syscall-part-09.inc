Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_inotify_1init1(
        JNIEnv *env, jclass clazz, jint javaFlags) {
    int flags = javaFlags;
    int fd = TEMP_FAILURE_RETRY(inotify_init1(flags));
    if (errno) {
        throwSyscallException(env, "inotify_init1");
        return NULL;
    }
    return newFileDescriptor(env, fd);
}

static jobject newStructInotifyEvent(JNIEnv *env, const struct inotify_event *event) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getStructInotifyEventClass(env), "<init>",
                                 "(IIILme/zhanghai/android/files/provider/common/ByteString;)V");
    }
    jint wd = event->wd;
    jint mask = (jint) event->mask;
    jint cookie = (jint) event->cookie;
    jobject name;
    size_t nameLength = strlen(event->name);
    if (nameLength) {
        name = newByteString(env, event->name, nameLength);
        if (!name) {
            return NULL;
        }
    } else {
        name = NULL;
    }
    return (*env)->NewObject(env, getStructInotifyEventClass(env), constructor, wd, mask, cookie,
            name);
}

JNIEXPORT jobjectArray JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_inotify_1get_1events(
        JNIEnv *env, jclass clazz, jbyteArray javaBuffer, jint javaOffset, jint javaLength) {
    void *buffer = (*env)->GetByteArrayElements(env, javaBuffer, NULL);
    size_t offset = (size_t) javaOffset;
    size_t length = (size_t) javaLength;
    char *bufferStart = (char *) buffer + offset;
    char *bufferEnd = bufferStart + length;
    jsize javaEventsLength = 0;
    for (char *eventStart = bufferStart; eventStart < bufferEnd; ) {
        struct inotify_event *event = (struct inotify_event *) eventStart;
        ++javaEventsLength;
        eventStart += sizeof(struct inotify_event) + event->len;
    }
    jobjectArray javaEvents = (*env)->NewObjectArray(env, javaEventsLength,
            getStructInotifyEventClass(env), NULL);
    if (!javaEvents) {
        (*env)->ReleaseByteArrayElements(env, javaBuffer, buffer, JNI_ABORT);
        return NULL;
    }
    jsize javaIndex = 0;
    for (char *eventStart = bufferStart; eventStart < bufferEnd; ) {
        struct inotify_event *event = (struct inotify_event *) eventStart;
        jobject javaEvent = newStructInotifyEvent(env, event);
        if (!javaEvent) {
            (*env)->DeleteLocalRef(env, javaEvents);
            (*env)->ReleaseByteArrayElements(env, javaBuffer, buffer, JNI_ABORT);
            return NULL;
        }
        (*env)->SetObjectArrayElement(env, javaEvents, javaIndex, javaEvent);
        (*env)->DeleteLocalRef(env, javaEvent);
        ++javaIndex;
        eventStart += sizeof(struct inotify_event) + event->len;
    }
    (*env)->ReleaseByteArrayElements(env, javaBuffer, buffer, JNI_ABORT);
    return javaEvents;
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_inotify_1rm_1watch(
        JNIEnv *env, jclass clazz, jobject javaFd, jint javaWd) {
    int fd = getFdFromFileDescriptor(env, javaFd);
    uint32_t wd = (uint32_t) javaWd;
    TEMP_FAILURE_RETRY(inotify_rm_watch(fd, wd));
    if (errno) {
        throwSyscallException(env, "inotify_rm_watch");
    }
}

JNIEXPORT jint JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_ioctl_1int(
        JNIEnv* env, jclass clazz, jobject javaFd, jint javaRequest, jobject javaArgument) {
    int fd = getFdFromFileDescriptor(env, javaFd);
    int request = javaRequest;
    int argument = 0;
    int* argumentPointer = NULL;
    if (javaArgument) {
        argument = (*env)->GetIntField(env, javaArgument, getInt32RefValueField(env));
        argumentPointer = &argument;
    }
    int result = TEMP_FAILURE_RETRY(ioctl(fd, request, argumentPointer));
    if (errno) {
        throwSyscallException(env, "ioctl");
        return 0;
    }
    if (javaArgument) {
        (*env)->SetIntField(env, javaArgument, getInt32RefValueField(env), argument);
    }
    return result;
}

JNIEXPORT void JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_lchown(
        JNIEnv *env, jclass clazz, jobject javaPath, jint javaUid, jint javaGid) {
    char *path = mallocStringFromByteString(env, javaPath);
    uid_t uid = (uid_t) javaUid;
    gid_t gid = (gid_t) javaGid;
    TEMP_FAILURE_RETRY(lchown(path, uid, gid));
    free(path);
    if (errno) {
        throwSyscallException(env, "lchown");
    }
}

JNIEXPORT jbyteArray JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_lgetxattr(
