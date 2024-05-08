typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef long i64;
typedef unsigned long u64;

u64 syscall0(u64 scid);
u64 syscall1(u64 scid, u64 a1);
u64 syscall2(u64 scid, u64 a1, u64 a2);
u64 syscall3(u64 scid, u64 a1, u64 a2, u64 a3);
u64 syscall4(u64 scid, u64 a1, u64 a2, u64 a3, u64 a4);
u64 syscall5(u64 scid, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5);
u64 syscall6(u64 scid, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 a6);

enum syscall {
    SYS_READ = 0,
    SYS_WRITE = 1,
    SYS_OPEN = 2,
    SYS_CLOSE = 3,
    SYS_POLL = 7,
    SYS_MMAP = 9,
    SYS_EXIT = 60,
    SYS_CLOCK_GETTIME = 228,
};

enum error_code {
    EINTR = 4,
};

static i32 syscall_error(u64 return_value) {
    if (return_value > -4096UL) {
        return (i32)(-return_value);
    }
    return 0;
}

static i64 read(i32 fd, char *bytes, i64 bytes_len) {
    u64 return_value;
    i32 error;
    do {
        return_value = syscall3(SYS_READ, (u64)fd, (u64)bytes, (u64)bytes_len);
        error = syscall_error(return_value);
    } while (error == EINTR);
    if (error != 0) {
        return -error;
    }
    return (i64)return_value;
}

static i64 write(i32 fd, char *bytes, i64 bytes_len) {
    u64 return_value;
    i32 error;
    do {
        return_value = syscall3(SYS_WRITE, (u64)fd, (u64)bytes, (u64)bytes_len);
        error = syscall_error(return_value);
    } while (error == EINTR);
    if (error != 0) {
        return -error;
    }
    return (i64)return_value;
}

enum open_mode {
    O_RDONLY = 0,
    O_WRONLY = 1,
    O_RDWR = 2,
};

static i32 open(char *fname, i32 mode, i32 flags) {
    u64 return_value;
    i32 error;
    do {
        return_value = syscall3(SYS_OPEN, (u64)fname, (u64)mode, (u64)flags);
        error = syscall_error(return_value);
    } while (error == EINTR);

    if (error != 0) {
        return -error;
    }
    return (i32)return_value;
}

static i32 close(i32 fd) {
    u64 return_value;
    i32 error;
    do {
        return_value = syscall1(SYS_CLOSE, (u64)fd);
        error = syscall_error(return_value);
    } while (error == EINTR);
    return error;
}

enum poll_event {
    POLLIN = 1,
};

struct pollfd {
    i32 fd;
    i16 events;
    i16 revents;
};

static i32 poll(struct pollfd *fds, i64 fds_len, i32 time_ms) {
    u64 return_value;
    i32 error;
    do {
        return_value = syscall3(SYS_POLL, (u64)fds, (u64)fds_len, (u64)time_ms);
        error = syscall_error(return_value);
    } while (error == EINTR);
    if (error != 0) {
        return -error;
    }
    return (i32)return_value;
}

enum mmap_prot {
    PROT_READ = 1,
    PROT_WRITE = 2,
};

enum mmap_flag {
    MAP_SHARED = 0x01,
    MAP_ANONYMOUS = 0x20,
};

static void *mmap(
    void *hint,
    i64 size,
    i32 prot,
    i32 flags,
    i32 fd,
    i64 offset
) {
    u64 return_value = syscall6(
        SYS_MMAP,
        (u64)hint,
        (u64)size,
        (u64)prot,
        (u64)flags,
        (u64)fd,
        (u64)offset
    );
    i32 error = syscall_error(return_value);
    if (error != 0) {
        return 0;
    }
    return (void *)return_value;
}

static void exit(i32 error_code) {
    syscall1(SYS_EXIT, (u64)error_code);
}

enum clock_id {
    CLOCK_MONOTONIC = 1,
};

struct timespec {
    i64 sec;
    i64 nsec;
};

static i32 clock_gettime(i32 clock_id, struct timespec *timespec) {
    u64 return_value = syscall2(
        SYS_CLOCK_GETTIME,
        (u64)clock_id,
        (u64)timespec
    );
    return syscall_error(return_value);
}

static i64 time_since_ns(struct timespec *end, struct timespec *start) {
    i64 seconds = end->sec - start->sec;
    return (seconds * 1000L * 1000L * 1000L) + end->nsec - start->nsec;
}

enum std_fd {
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2,
};

struct arena {
    char *start;
    char *end;
};

void *alloc(struct arena *arena, i64 size);

enum main_error {
    MAIN_ERROR_NONE = 0,
    MAIN_ERROR_MMAP,
};

i32 main(i32 argc, char **argv) {
    i64 arena_size = 2000 * 4096;
    char *mem = mmap(
        0,
        arena_size,
        PROT_WRITE | PROT_READ,
        MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );
    if (mem == 0) {
        return MAIN_ERROR_MMAP;
    }

    struct arena arena = { .start = mem, .end = mem + arena_size };
    char *buf = 0;
    i64 buf_len = 0;

    return MAIN_ERROR_NONE;
}

void _cstart(i32 argc, char **argv) {
    exit(main(argc, argv));
}

