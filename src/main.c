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
    SYS_IOCTL = 16,
    SYS_EXIT = 60,
    SYS_GETDENTS = 78,
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

enum ioctl_dir {
    IOCTL_WRITE = 1,
    IOCTL_READ = 2,
    IOCTL_RDWR = 3,
};

static i32 ioctl(i32 fd, u32 dir, u32 type, u32 number, u32 size, char *arg) {
    u32 number_bits = number & 0xff;
    u32 type_bits = (type & 0xff) << 8;
    u32 size_bits = (size & 0x3fff) << 16;
    u32 dir_bits = (dir & 0x3) << 30;
    u32 request = dir_bits | size_bits | type_bits | number_bits;

    u64 return_value;
    i32 error;
    do {
        return_value = syscall3(SYS_IOCTL, (u64)fd, (u64)request, (u64)arg);
        error = syscall_error(return_value);
    } while (error == EINTR);
    return error;
}

static void exit(i32 error_code) {
    syscall1(SYS_EXIT, (u64)error_code);
}

struct dirent {
    u64 ino;
    u64 off;
    u16 reclen;
    char name[];
};

static i64 getdents(i32 fd, struct dirent *dents, i64 dents_size) {
    u64 return_value = syscall3(
        SYS_GETDENTS,
        (u64)fd,
        (u64)dents,
        (u64)dents_size
    );
    i32 error = syscall_error(return_value);
    if (error != 0) {
        return -error;
    }
    return (i64)return_value;
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

enum ioctl_type {
    IOCTL_EV = (i32)'E',
};

enum ev_ioctl {
    EV_IOCTL_GET_BIT = 0x20,
    EV_IOCTL_GET_KEY = 0x21,
    EV_IOCTL_GRAB = 0x90,
};

enum ev_bits {
    EV_KEY = 0x1,
    EV_MAX = 0x1f,
};

enum ev_key_bits {
    KEY_ESC = 1,
    KEY_W = 17,
    KEY_A = 30,
    KEY_S = 31,
    KEY_D = 32,
    KEY_MAX = 0x2ff
};

static i32 test_bit(char *bytes, i32 len, i32 bit_num) {
    i32 byte_index = bit_num / 8;
    i32 bit_index = bit_num % 8;
    if (byte_index >= len) {
        return 0;
    }

    return (bytes[byte_index] & (1 << bit_index)) != 0;
}

static i32 is_keyboard(i32 fd) {
    char evio_bits[EV_MAX / 8 + 1];
    i32 error = ioctl(
        fd,
        IOCTL_READ,
        IOCTL_EV,
        EV_IOCTL_GET_BIT,
        sizeof(evio_bits),
        evio_bits
    );
    if (error != 0) {
        return 0;
    }
    if (!test_bit(evio_bits, sizeof(evio_bits), EV_KEY)) {
        return 0;
    }

    char evio_key_bits[KEY_MAX / 8 + 1];
    error = ioctl(
        fd,
        IOCTL_READ,
        IOCTL_EV,
        EV_IOCTL_GET_KEY,
        sizeof(evio_key_bits),
        evio_key_bits
    );
    if (error != 0) {
        return 0;
    }
    if (
        test_bit(evio_key_bits, sizeof(evio_key_bits), KEY_ESC) &&    
        test_bit(evio_key_bits, sizeof(evio_key_bits), KEY_W) &&
        test_bit(evio_key_bits, sizeof(evio_key_bits), KEY_A) &&
        test_bit(evio_key_bits, sizeof(evio_key_bits), KEY_S) &&
        test_bit(evio_key_bits, sizeof(evio_key_bits), KEY_D)
    ) {
        return 1;
    }
    return 0;
}

static i32 open_keyboards(
    struct arena temp_arena,
    i32 *keyboards,
    i32 keyboards_capacity
) {
    char input_dir[] = "/dev/input";
    i32 input_dir_fd = open(input_dir, O_RDONLY, 0);
    if (input_dir_fd < 0) {
        return -1;
    }

    void *dents = alloc(&temp_arena, 1024);
    char path_buffer[sizeof(input_dir) + 1024];
    for (i32 i = 0; i < sizeof(input_dir); ++i) {
        path_buffer[i] = input_dir[i];
    }
    path_buffer[sizeof(input_dir) - 1] = '/';
    char *name_buffer = &path_buffer[sizeof(input_dir)];

    i64 dents_pos = 0;
    i64 dents_len = 0;
    i32 keyboards_len = 0;
    while (keyboards_len < keyboards_capacity) {
        if (dents_pos >= dents_len) {
            dents_len = getdents(input_dir_fd, dents, 1024);
            if (dents_len <= 0) {
                break;
            }
        }

        struct dirent *dent = (void *)((char *)dents + dents_pos);
        i32 dent_name_len = dent->reclen - (dent->name - (char *)dent);
        for (i32 i = 0; i < dent_name_len; ++i) {
            name_buffer[i] = dent->name[i];
        }
        dents_pos += dent->reclen;

        i32 keyboard_fd = open(path_buffer, O_RDONLY, 0);
        if (keyboard_fd >= 0 && !is_keyboard(keyboard_fd)) {
            close(keyboard_fd);
            continue;
        }

        i32 error = ioctl(
            keyboard_fd,
            IOCTL_WRITE,
            IOCTL_EV,
            EV_IOCTL_GRAB,
            sizeof(u32),
            (char *)1
        );
        if (error != 0) {
            close(keyboard_fd);
            continue;
        }

        keyboards[keyboards_len] = keyboard_fd;
        keyboards_len += 1;
    }

    close(input_dir_fd);
    return keyboards_len;
}

struct input_event {
    struct timespec time;
    u16 type;
    u16 code;
    i32 value;
};

enum main_error {
    MAIN_ERROR_NONE = 0,
    MAIN_ERROR_MMAP,
    MAIN_ERROR_WRITE_STDOUT,
    MAIN_ERROR_CLOCK_GETTIME,
    MAIN_ERROR_POLL,
    MAIN_ERROR_OPEN_KEYBOARD,
    MAIN_ERROR_READ_KEYBOARD,
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

    char greeting[] = "Press ESC exit.\n";
    i64 len = write(STDOUT, greeting, sizeof(greeting) - 1);
    if (len < 0) {
        return MAIN_ERROR_WRITE_STDOUT;
    }

    struct timespec last, now;
    i32 error = clock_gettime(CLOCK_MONOTONIC, &last);
    if (error) {
        return MAIN_ERROR_CLOCK_GETTIME;
    }

    i32 keyboards[32];
    i32 keyboards_len = open_keyboards(
        arena,
        keyboards,
        sizeof(keyboards) / sizeof(*keyboards)
    );
    if (keyboards_len <= 0) {
        return MAIN_ERROR_OPEN_KEYBOARD;
    }

    struct input_event keyboard_events[32];
    struct pollfd keyboard_pollfds[32];
    for (i32 i = 0; i < keyboards_len; ++i) {
        keyboard_pollfds[i].fd = keyboards[i];
        keyboard_pollfds[i].events = POLLIN;
    }

    while (1) {
        i64 events = poll(keyboard_pollfds, keyboards_len, 0);
        if (events < 0) {
            return MAIN_ERROR_POLL;
        }

        for (i32 i = 0; i < keyboards_len; ++i) {
            if (keyboard_pollfds[i].revents == 0) {
                continue;
            }
            i32 keyboard_fd = keyboard_pollfds[i].fd;

            i64 len = read(
                keyboard_fd,
                (char *)keyboard_events,
                sizeof(keyboard_events)
            );
            if (len < 0) {
                return MAIN_ERROR_READ_KEYBOARD;
            }

            for (i32 i = 0; i < len / (i64)sizeof(*keyboard_events); ++i) {
                struct input_event *keyboard_event = &keyboard_events[i];
                if (keyboard_event->type == 1 && keyboard_event->value == 1) {
                    switch (keyboard_event->code) {
                        case KEY_ESC:
                            return MAIN_ERROR_NONE;
                        default:
                            continue;
                    }
                }
            }
        }

        i32 error = clock_gettime(CLOCK_MONOTONIC, &now);
        if (error) {
            return MAIN_ERROR_CLOCK_GETTIME;
        }

        if (time_since_ns(&now, &last) >= 1000L * 1000L * 1000L) {
            last = now;

            len = write(STDOUT, ".", 1);
            if (len < 0) {
                return MAIN_ERROR_WRITE_STDOUT;
            }
        }
    }

    return MAIN_ERROR_NONE;
}

void _cstart(i32 argc, char **argv) {
    exit(main(argc, argv));
}

