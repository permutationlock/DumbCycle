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
    MAIN_ERROR_READ_NAME,
};

i32 main(i32 argc, char **argv) {
    i64 len;
    i32 name_fd = open("name.txt", O_RDONLY, 0);
    if (name_fd < 0) {
        char question[] = "What is your name?\n";
        len = write(STDOUT, question, sizeof(question));
        if (len < 0){
            return MAIN_ERROR_WRITE_STDOUT;
        }
        name_fd = STDIN;
    }

    char name[255];
    i64 name_len = read(name_fd, name, sizeof(name));
    if (name_len < 0) {
        return MAIN_ERROR_READ_NAME;
    }

    char greeting1[] = "Hello ";
    len = write(STDOUT, greeting1, sizeof(greeting1) - 1);
    if (len < 0) {
        return MAIN_ERROR_WRITE_STDOUT;
    }

    len = write(STDOUT, name, name_len);
    if (len < 0) {
        return MAIN_ERROR_WRITE_STDOUT;
    }

    return MAIN_ERROR_NONE;
}

void _cstart(i32 argc, char **argv) {
    exit(main(argc, argv));
}

