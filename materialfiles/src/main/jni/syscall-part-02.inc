    if (!byteStringClass) {
        byteStringClass = findClass(env, "me/zhanghai/android/files/provider/common/ByteString");
    }
    return byteStringClass;
}

static jfieldID getByteStringBytesField(JNIEnv *env) {
    static jfieldID byteStringBytesField = NULL;
    if (!byteStringBytesField) {
        byteStringBytesField = findField(env, getByteStringClass(env), "bytes", "[B");
    }
    return byteStringBytesField;
}

static jclass getFileDescriptorClass(JNIEnv *env) {
    static jclass fileDescriptorClass = NULL;
    if (!fileDescriptorClass) {
        fileDescriptorClass = findClass(env, "java/io/FileDescriptor");
    }
    return fileDescriptorClass;
}

static jfieldID getFileDescriptorDescriptorField(JNIEnv *env) {
    static jfieldID fileDescriptorDescriptorField = NULL;
    if (!fileDescriptorDescriptorField) {
        fileDescriptorDescriptorField = findField(env, getFileDescriptorClass(env), "descriptor",
                                                  "I");
    }
    return fileDescriptorDescriptorField;
}

static jclass getInt32RefClass(JNIEnv *env) {
    static jclass int32RefClass = NULL;
    if (!int32RefClass) {
        int32RefClass = findClass(env, "me/zhanghai/android/files/provider/linux/syscall/Int32Ref");
    }
    return int32RefClass;
}

static jfieldID getInt32RefValueField(JNIEnv *env) {
    static jfieldID int32RefValueField = NULL;
    if (!int32RefValueField) {
        int32RefValueField = findField(env, getInt32RefClass(env), "value", "I");
    }
    return int32RefValueField;
}

static jclass getInt64RefClass(JNIEnv *env) {
    static jclass int64RefClass = NULL;
    if (!int64RefClass) {
        int64RefClass = findClass(env, "android/system/Int64Ref");
    }
    return int64RefClass;
}

static jfieldID getInt64RefValueField(JNIEnv *env) {
    static jfieldID int64RefValueField = NULL;
    if (!int64RefValueField) {
        int64RefValueField = findField(env, getInt64RefClass(env), "value", "J");
    }
    return int64RefValueField;
}

static jclass getStructDirentClass(JNIEnv *env) {
    static jclass structStatClass = NULL;
    if (!structStatClass) {
        structStatClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/StructDirent");
    }
    return structStatClass;
}

static jclass getStructGroupClass(JNIEnv *env) {
    static jclass structGroupClass = NULL;
    if (!structGroupClass) {
        structGroupClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/StructGroup");
    }
    return structGroupClass;
}

static jclass getStructInotifyEventClass(JNIEnv *env) {
    static jclass structInotifyEventClass = NULL;
    if (!structInotifyEventClass) {
        structInotifyEventClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/StructInotifyEvent");
    }
    return structInotifyEventClass;
}

static jclass getStructMntentClass(JNIEnv *env) {
    static jclass structMntentClass = NULL;
    if (!structMntentClass) {
        structMntentClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/StructMntent");
    }
    return structMntentClass;
}

static jfieldID getStructMntentMntOptsField(JNIEnv *env) {
    static jfieldID structMntentMntOptsField = NULL;
    if (!structMntentMntOptsField) {
        structMntentMntOptsField = findField(env, getStructMntentClass(env), "mnt_opts",
                "Lme/zhanghai/android/files/provider/common/ByteString;");
    }
    return structMntentMntOptsField;
}

static jclass getStructPasswdClass(JNIEnv *env) {
    static jclass structPasswdClass = NULL;
    if (!structPasswdClass) {
        structPasswdClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/StructPasswd");
    }
    return structPasswdClass;
}

static jclass getStructStatClass(JNIEnv *env) {
    static jclass structStatClass = NULL;
    if (!structStatClass) {
