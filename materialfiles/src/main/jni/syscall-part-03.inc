        structStatClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/StructStat");
    }
    return structStatClass;
}

static jclass getStructStatVfsClass(JNIEnv *env) {
    static jclass structStatVfsClass = NULL;
    if (!structStatVfsClass) {
        structStatVfsClass = findClass(env, "android/system/StructStatVfs");
    }
    return structStatVfsClass;
}

static jclass getStructTimespecClass(JNIEnv *env) {
    static jclass structTimespecClass = NULL;
    if (!structTimespecClass) {
        structTimespecClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/StructTimespec");
    }
    return structTimespecClass;
}

static jfieldID getStructTimespecTvSecField(JNIEnv *env) {
    static jfieldID structTimespecTvSecField = NULL;
    if (!structTimespecTvSecField) {
        structTimespecTvSecField = findField(env, getStructTimespecClass(env), "tv_sec", "J");
    }
    return structTimespecTvSecField;
}

static jfieldID getStructTimespecTvNsecField(JNIEnv *env) {
    static jfieldID structTimespecTvNsecField = NULL;
    if (!structTimespecTvNsecField) {
        structTimespecTvNsecField = findField(env, getStructTimespecClass(env), "tv_nsec", "J");
    }
    return structTimespecTvNsecField;
}

static void throwException(JNIEnv *env, jclass exceptionClass, jmethodID constructor3,
                           jmethodID constructor2, const char *functionName, int error) {
    jthrowable cause = NULL;
    if ((*env)->ExceptionCheck(env)) {
        cause = (*env)->ExceptionOccurred(env);
        (*env)->ExceptionClear(env);
    }
    jstring detailMessage = (*env)->NewStringUTF(env, functionName);
    if (!detailMessage) {
        // Not really much we can do here. We're probably dead in the water,
        // but let's try to stumble on...
        (*env)->ExceptionClear(env);
    }
    jobject exception;
    if (cause) {
        exception = (*env)->NewObject(env, exceptionClass, constructor3, detailMessage, error,
                cause);
    } else {
        exception = (*env)->NewObject(env, exceptionClass, constructor2, detailMessage, error);
    }
    (*env)->Throw(env, exception);
    if (detailMessage) {
        (*env)->DeleteLocalRef(env, detailMessage);
    }
}

static void throwSyscallException(JNIEnv* env, const char* functionName) {
    int error = errno;
    static jmethodID constructor3 = NULL;
    if (!constructor3) {
        constructor3 = findMethod(env, getSyscallExceptionClass(env), "<init>",
                "(Ljava/lang/String;ILjava/lang/Throwable;)V");
    }
    static jmethodID constructor2 = NULL;
    if (!constructor2) {
        constructor2 = findMethod(env, getSyscallExceptionClass(env), "<init>",
                "(Ljava/lang/String;I)V");
    }
    throwException(env, getSyscallExceptionClass(env), constructor3, constructor2, functionName,
                   error);
}

static char *mallocStringFromByteString(JNIEnv *env, jobject javaByteString) {
    jbyteArray javaBytes = (*env)->GetObjectField(env, javaByteString, getByteStringBytesField(
            env));
    void *bytes = (*env)->GetByteArrayElements(env, javaBytes, NULL);
    jsize javaLength = (*env)->GetArrayLength(env, javaBytes);
    size_t length = (size_t) javaLength;
    char *string = malloc(length + 1);
    memcpy(string, bytes, length);
    (*env)->ReleaseByteArrayElements(env, javaBytes, bytes, JNI_ABORT);
    (*env)->DeleteLocalRef(env, javaBytes);
    string[length] = '\0';
    return string;
}

static jobject newByteString(JNIEnv *env, const void *bytes, size_t length) {
    static jmethodID constructor = NULL;
    if (!constructor) {
        constructor = findMethod(env, getByteStringClass(env), "<init>", "([B)V");
    }
    jsize javaLength = (jsize) length;
    jbyteArray javaBytes = (*env)->NewByteArray(env, javaLength);
    if (!javaBytes) {
        return NULL;
    }
    (*env)->SetByteArrayRegion(env, javaBytes, 0, javaLength, bytes);
    jobject javaByteString = (*env)->NewObject(env, getByteStringClass(env), constructor,
            javaBytes);
    (*env)->DeleteLocalRef(env, javaBytes);
    return javaByteString;
}

static jobject newByteStringFromString(JNIEnv *env, const char *string) {
    return newByteString(env, string, strlen(string));
}

static int getFdFromFileDescriptor(JNIEnv *env, jobject javaFileDescriptor) {
    return (*env)->GetIntField(env, javaFileDescriptor, getFileDescriptorDescriptorField(env));
}

