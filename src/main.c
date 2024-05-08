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
    SYS_WRITE = 1,
    SYS_EXIT = 60,
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

static void exit(i32 error_code) {
    syscall1(SYS_EXIT, (u64)error_code);
}

enum std_fd {
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2,
};

enum main_error {
    MAIN_ERROR_NONE = 0,
    MAIN_ERROR_WRITE_STDOUT,
};

i32 main(i32 argc, char **argv) {
    char greeting[] = "Hello, World!\n";
    i64 len = write(STDOUT, greeting, sizeof(greeting));
    if (len < 0){
        return MAIN_ERROR_WRITE_STDOUT;
    }
    return MAIN_ERROR_NONE;
}

void _cstart(i32 argc, char **argv) {
    exit(main(argc, argv));
}

