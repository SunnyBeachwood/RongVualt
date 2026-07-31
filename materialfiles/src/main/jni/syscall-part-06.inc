        if (!group) {
            return NULL;
        }
        if (group->gr_name[0] == 'o' && group->gr_name[1] == 'e' && group->gr_name[2] == 'm'
            && group->gr_name[3] == '_') {
            continue;
        }
        if (group->gr_name[0] == 'u' && (group->gr_name[1] >= '0' && group->gr_name[1] <= '9')) {
            return NULL;
        }
        if (group->gr_name[0] == 'a' && group->gr_name[1] == 'l' && group->gr_name[2] == 'l'
            && group->gr_name[3] == '_' && group->gr_name[4] == 'a'
            && (group->gr_name[5] >= '0' && group->gr_name[5] <= '9')) {
            return NULL;
        }
        return newStructGroup(env, group);
    }
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_getgrgid(
        JNIEnv *env, jclass clazz, jint javaGid) {
#if __ANDROID_API__ >= __ANDROID_API_N__
    gid_t gid = (gid_t) javaGid;
    size_t bufferSize = (size_t) sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufferSize == -1) {
        // See `man 3 getpwnam`
        bufferSize = 16384;
    }
    //char buffer[bufferSize] = {};
    char buffer[bufferSize];
    struct group group = {};
    struct group *result = NULL;
    errno = TEMP_FAILURE_RETRY_E(getgrgid_r(gid, &group, buffer, bufferSize, &result));
    if (errno) {
        throwSyscallException(env, "getgrgid_r");
        return NULL;
    }
    if (!result) {
        return NULL;
    }
    return newStructGroup(env, result);
#else
    gid_t gid = (gid_t) javaGid;
    struct group *result = TEMP_FAILURE_RETRY_N(getgrgid(gid));
    if (errno) {
        throwSyscallException(env, "getgrgid");
        return NULL;
    }
    if (!result) {
        return NULL;
    }
    return newStructGroup(env, result);
#endif
}

JNIEXPORT jobject JNICALL
Java_me_zhanghai_android_files_provider_linux_syscall_Syscall_getgrnam(
        JNIEnv *env, jclass clazz, jobject javaName) {
#if __ANDROID_API__ >= __ANDROID_API_N__
    char *name = mallocStringFromByteString(env, javaName);
    size_t bufferSize = (size_t) sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufferSize == -1) {
        // See `man 3 getpwnam`
        bufferSize = 16384;
    }
    //char buffer[bufferSize] = {};
    char buffer[bufferSize];
    struct group group = {};
    struct group *result = NULL;
    errno = TEMP_FAILURE_RETRY_E(getgrnam_r(name, &group, buffer, bufferSize, &result));
    free(name);
    if (errno) {
        throwSyscallException(env, "getgrnam_r");
        return NULL;
    }
    if (!result) {
        return NULL;
    }
    return newStructGroup(env, result);
#else
    char *name = mallocStringFromByteString(env, javaName);
    struct group *result = TEMP_FAILURE_RETRY_N(getgrnam(name));
    free(name);
    if (errno) {
        throwSyscallException(env, "getgrnam");
        return NULL;
    }
    if (!result) {
        return NULL;
    }
    return newStructGroup(env, result);
#endif
}

#if __ANDROID_API__ < __ANDROID_API_L_MR1__
// https://android.googlesource.com/platform/bionic/+/master/libc/bionic/mntent.cpp
static struct mntent* _getmntent_r(FILE* fp, struct mntent* e, char* buf, int buf_len) {
    memset(e, 0, sizeof(*e));
    while (fgets(buf, buf_len, fp) != NULL) {
        // Entries look like "proc /proc proc rw,nosuid,nodev,noexec,relatime 0 0".
        // That is: mnt_fsname mnt_dir mnt_type mnt_opts 0 0.
        int fsname0, fsname1, dir0, dir1, type0, type1, opts0, opts1;
        if (sscanf(buf, " %n%*s%n %n%*s%n %n%*s%n %n%*s%n %d %d",
                   &fsname0, &fsname1, &dir0, &dir1, &type0, &type1, &opts0, &opts1,
                   &e->mnt_freq, &e->mnt_passno) == 2) {
            e->mnt_fsname = &buf[fsname0];
            buf[fsname1] = '\0';
            e->mnt_dir = &buf[dir0];
            buf[dir1] = '\0';
            e->mnt_type = &buf[type0];
            buf[type1] = '\0';
            e->mnt_opts = &buf[opts0];
            buf[opts1] = '\0';
            return e;
        }
    }
    return NULL;
}
#endif
