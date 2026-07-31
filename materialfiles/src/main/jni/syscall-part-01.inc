/*
 * Copyright (c) 2018 Hai Zhang <dreaming.in.code.zh@gmail.com>
 * All Rights Reserved.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <mntent.h>
#include <pwd.h>
#include <sys/inotify.h>
#include <sys/mount.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

#include <jni.h>

#include <android/log.h>

#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LOG_TAG "syscall"

#undef TEMP_FAILURE_RETRY
// Checks errno when return value is -1.
#define TEMP_FAILURE_RETRY(exp) ({ \
    __typeof__(exp) _rc; \
    do { \
        errno = 0; \
        _rc = (exp); \
    } while (_rc == -1 && errno == EINTR); \
    if (_rc != -1) { \
        errno = 0; \
    } \
    _rc; })

// Checks return value as errno.
#define TEMP_FAILURE_RETRY_E(exp) ({ \
    __typeof__(exp) _rc; \
    do { \
        _rc = (exp); \
    } while (_rc == EINTR); \
    _rc; })

// Checks errno when return value is NULL.
#define TEMP_FAILURE_RETRY_N(exp) ({ \
    __typeof__(exp) _rc; \
    do { \
        errno = 0; \
        _rc = (exp); \
    } while (!_rc && errno == EINTR); \
    if (_rc) { \
        errno = 0; \
    } \
    _rc; })

// Always checks errno and ignores return value.
#define TEMP_FAILURE_RETRY_V(exp) ({ \
    do { \
        errno = 0; \
        (exp); \
    } while (errno == EINTR); })

static jclass findClass(JNIEnv *env, const char *name) {
    jclass localClass = (*env)->FindClass(env, name);
    if (!localClass) {
        ALOGE("Failed to find class '%s'", name);
        abort();
    }
    jclass globalClass = (*env)->NewGlobalRef(env, localClass);
    (*env)->DeleteLocalRef(env, localClass);
    if (!globalClass) {
        ALOGE("Failed to create a global reference for '%s'", name);
        abort();
    }
    return globalClass;
}

static jfieldID findField(JNIEnv *env, jclass clazz, const char *name, const char *signature) {
    jfieldID field = (*env)->GetFieldID(env, clazz, name, signature);
    if (!field) {
        ALOGE("Failed to find field '%s' '%s'", name, signature);
        abort();
    }
    return field;
}

static jmethodID findMethod(JNIEnv *env, jclass clazz, const char *name, const char *signature) {
    jmethodID method = (*env)->GetMethodID(env, clazz, name, signature);
    if (!method) {
        ALOGE("Failed to find method '%s' '%s'", name, signature);
        abort();
    }
    return method;
}

static jclass getSyscallExceptionClass(JNIEnv *env) {
    static jclass syscallExceptionClass = NULL;
    if (!syscallExceptionClass) {
        syscallExceptionClass = findClass(env,
                "me/zhanghai/android/files/provider/linux/syscall/SyscallException");
    }
    return syscallExceptionClass;
}

static jclass getByteStringClass(JNIEnv *env) {
    static jclass byteStringClass = NULL;
