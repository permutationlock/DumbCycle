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
    SYS_OPENAT = 257,
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

static void *mmap(void *hint, i64 size, i32 prot, i32 flags, i32 fd, i64 offset) {
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
    u64 return_value = syscall2(SYS_CLOCK_GETTIME, (u64)clock_id, (u64)timespec);
    return syscall_error(return_value);
}

static i64 time_since_ns(struct timespec *end, struct timespec *start) {
    i64 seconds = end->sec - start->sec;
    return (seconds * 1000L * 1000L * 1000L) + end->nsec - start->nsec;
}

static i32 openat(i32 dfd, char *fname, i32 mode, i32 flags) {
    u64 return_value;
    i32 error;
    do {
        return_value = syscall4(
            SYS_OPENAT,
            (u64)dfd,
            (u64)fname,
            (u64)mode,
            (u64)flags
        );
        error = syscall_error(return_value);
    } while (error == EINTR);

    if (error != 0) {
        return -error;
    }
    return (i32)return_value;
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
    IOCTL_DRM = (i32)'d',
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
    KEY_MAX = 0x2ff,
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
        dents_pos += dent->reclen;

        i32 keyboard_fd = openat(input_dir_fd, dent->name, O_RDONLY, 0);
        if (keyboard_fd < 0 || !is_keyboard(keyboard_fd)) {
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

enum drm_ioctl {
    DRM_IOCTL_MODE_GET_RESOURCES = 0xa0,
    DRM_IOCTL_MODE_GET_CONNECTOR = 0xa7,
    DRM_IOCTL_MODE_GET_ENCODER = 0xa6,
    DRM_IOCTL_MODE_ADD_FB = 0xae,
    DRM_IOCTL_MODE_CREATE_DUMB = 0xb2,
    DRM_IOCTL_MODE_MAP_DUMB = 0xb3,
    DRM_IOCTL_MODE_GET_CRTC = 0xa1,
    DRM_IOCTL_MODE_SET_CRTC = 0xa2,
    DRM_IOCTL_MODE_PAGE_FLIP = 0xb0,
};

struct drm_mode_resources {
    u32 *fbs;
    u32 *crtcs;
    u32 *connectors;
    u32 *encoders;

    u32 fbs_len;
    u32 crtcs_len;
    u32 connectors_len;
    u32 encoders_len;

    u32 min_width;
    u32 max_width;
    u32 min_height;
    u32 max_height;
};

static struct drm_mode_resources *drm_mode_get_resources(
    struct arena *arena,
    i32 fd
) {
    struct drm_mode_resources prev_res;
    struct drm_mode_resources *res;
    struct arena temp_arena;
    i32 error;

    do {
        temp_arena = *arena;
        res = alloc(&temp_arena, sizeof(*res));
        if (res == 0) {
            return 0;
        }
        error = ioctl(
            fd,
            IOCTL_RDWR,
            IOCTL_DRM,
            DRM_IOCTL_MODE_GET_RESOURCES,
            sizeof(*res),
            (char *)res
        );
        if (error != 0) {
            return 0;
        }

        prev_res = *res;

        if (res->fbs_len > 0) {
            res->fbs = alloc(&temp_arena, res->fbs_len * sizeof(*res->fbs));
            if (res->fbs == 0) {
                return 0;
            }
        }
        if (res->crtcs_len > 0) {
            res->crtcs = alloc(
                &temp_arena,
                res->crtcs_len * sizeof(*res->crtcs)
            );
            if (res->crtcs == 0) {
                return 0;
            }
        }
        if (res->connectors_len > 0) {
            res->connectors = alloc(
                &temp_arena,
                res->connectors_len * sizeof(*res->connectors)
            );
            if (res->connectors == 0) {
                return 0;
            }
        }
        if (res->encoders_len > 0) {
            res->encoders = alloc(
                &temp_arena,
                res->encoders_len * sizeof(*res->encoders)
            );
            if (res->encoders == 0) {
                return 0;
            }
        }

        error = ioctl(
            fd,
            IOCTL_RDWR,
            IOCTL_DRM,
            DRM_IOCTL_MODE_GET_RESOURCES,
            sizeof(*res),
            (char *)res
        );
        if (error != 0) {
            return 0;
        }
    } while (
        prev_res.fbs_len < res->fbs_len ||
        prev_res.crtcs_len < res->crtcs_len ||
        prev_res.connectors_len < res->connectors_len ||
        prev_res.encoders_len < res->encoders_len
    );

    *arena = temp_arena;
    return res;
}

enum drm_mode {
    DRM_MODE_CONNECTED = 1,
};

struct drm_mode_modeinfo {
    u32 clock;

    u16 hdisplay;
    u16 hsync_start;
    u16 hsync_end;
    u16 htotal;
    u16 hskew;

    u16 vdisplay;
    u16 vsync_start;
    u16 vsync_end;
    u16 vtotal;
    u16 vscan;

    u32 vrefresh;

    u32 flags;
    u32 type;
    char name[32];
};

struct drm_mode_connector {
    u32 *encoders;
    struct drm_mode_modeinfo *modes;
    u32 *props;
    u64 *prop_values;

    u32 modes_len;
    u32 props_len;
    u32 encoders_len;

    u32 encoder_id;
    u32 connector_id;

    u32 connector_type;
    u32 connector_type_id;
    u32 connection;
    u32 mm_width;
    u32 mm_height;
    u32 subpixel;
    u32 pad;
};

static struct drm_mode_connector *drm_mode_get_connector(
    struct arena *arena,
    i32 fd,
    u32 connector_id
) {
    struct drm_mode_connector prev_conn;
    struct drm_mode_connector *conn;
    struct arena temp_arena;
    i32 error;

    do {
        temp_arena = *arena;
        conn = alloc(&temp_arena, sizeof(*conn));
        if (conn == 0) {
            return 0;
        }
        conn->connector_id = connector_id;
        error = ioctl(
            fd,
            IOCTL_RDWR,
            IOCTL_DRM,
            DRM_IOCTL_MODE_GET_CONNECTOR,
            sizeof(*conn),
            (char *)conn
        );
        if (error != 0) {
            return 0;
        }

        prev_conn = *conn;

        if (conn->props_len > 0) {
            conn->props = alloc(
                &temp_arena,
                conn->props_len * sizeof(*conn->props)
            );
            conn->prop_values = alloc(
                &temp_arena,
                conn->props_len * sizeof(*conn->prop_values)
            );
            if (conn->props == 0 || conn->prop_values == 0) {
                return 0;
            }
        }
        if (conn->modes_len > 0) {
            conn->modes = alloc(
                &temp_arena,
                conn->modes_len * sizeof(*conn->modes)
            );
            if (conn->modes == 0) {
                return 0;
            }
        }
        if (conn->encoders_len > 0) {
            conn->encoders = alloc(
                &temp_arena,
                conn->encoders_len * sizeof(*conn->encoders)
            );
            if (conn->encoders == 0) {
                return 0;
            }
        }

        error = ioctl(
            fd,
            IOCTL_RDWR,
            IOCTL_DRM,
            DRM_IOCTL_MODE_GET_CONNECTOR,
            sizeof(*conn),
            (char *)conn
        );
        if (error != 0) {
            return 0;
        }
    } while (
        prev_conn.props_len < conn->props_len ||
        prev_conn.modes_len < conn->modes_len ||
        prev_conn.encoders_len < conn->encoders_len
    );

    *arena = temp_arena;
    return conn;
}

struct drm_mode_encoder {
    u32 encoder_id;
    u32 encoder_type;

    u32 crtc_id;

    u32 possible_crtcs;
    u32 possible_clones;
};

static struct drm_mode_encoder *drm_mode_get_encoder(
    struct arena *arena,
    i32 fd,
    u32 encoder_id
) {
    struct arena temp_arena = *arena;
    struct drm_mode_encoder *enc = 0;

    enc = alloc(&temp_arena, sizeof(*enc));
    if (enc == 0) {
        return 0;
    }

    enc->encoder_id = encoder_id;
    i32 error = ioctl(
        fd,
        IOCTL_RDWR,
        IOCTL_DRM,
        DRM_IOCTL_MODE_GET_ENCODER,
        sizeof(*enc),
        (char *)enc
    );
    if (error != 0) {
        return 0;
    }

    *arena = temp_arena;
    return enc;
}

struct drm_mode_create_dumb {
    u32 height;
    u32 width;
    u32 bpp;
    u32 flags;
    u32 handle;
    u32 pitch;
    u64 size;
};

struct drm_mode_map_dumb {
    u32 handle;
    u32 pad;
    i64 offset;
};

struct drm_mode_fb_cmd {
    u32 fb_id;
    u32 width;
    u32 height;
    u32 pitch;
    u32 bpp;
    u32 depth;
    u32 handle;
};

struct drm_mode_dumb_buffer {
    u32 width;
    u32 height;
    u32 stride;
    u32 handle;
    u32 fb_id;
    u32 *map;
    u64 size;
};

static struct drm_mode_dumb_buffer *drm_mode_create_dumb_buffer(
    struct arena *arena,
    i32 fd,
    u32 width,
    u32 height
) {
    struct drm_mode_create_dumb creq = {
        .width = width,
        .height = height,
        .bpp = 32,
    };
    i32 error = ioctl(
        fd,
        IOCTL_RDWR,
        IOCTL_DRM,
        DRM_IOCTL_MODE_CREATE_DUMB,
        sizeof(creq),
        (char *)&creq
    );
    if (error != 0) {
        return 0;
    }

    struct drm_mode_fb_cmd fb_cmd = {
        .width = width,
        .height = height,
        .pitch = creq.pitch,
        .bpp = 32,
        .depth = 24,
        .handle = creq.handle,
    };
    error = ioctl(
        fd,
        IOCTL_RDWR,
        IOCTL_DRM,
        DRM_IOCTL_MODE_ADD_FB,
        sizeof(fb_cmd),
        (char *)&fb_cmd
    );
    if (error != 0) {
        return 0;
    }

    struct drm_mode_map_dumb mreq = { .handle = creq.handle };
    error = ioctl(
        fd,
        IOCTL_RDWR,
        IOCTL_DRM,
        DRM_IOCTL_MODE_MAP_DUMB,
        sizeof(mreq),
        (char *)&mreq
    );
    if (error != 0) {
        return 0;
    }

    u32 *mem = mmap(
        0,
        (i64)creq.size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        mreq.offset
    );
    if (mem == 0) {
        return 0;
    }

    struct drm_mode_dumb_buffer *buf = alloc(arena, sizeof(*buf));
    buf->width = width;
    buf->height = height;
    buf->stride = creq.pitch / sizeof(u32);
    buf->size = creq.size / sizeof(u32);
    buf->handle = creq.handle;
    buf->map = mem;
    buf->fb_id = fb_cmd.fb_id;

    for (u64 i = 0; i < buf->size; ++i) {
        buf->map[i] = 0;
    }

    return buf;
}

struct drm_mode_crtc {
    u32 *set_connectors;
    u32 connectors_len;

    u32 crtc_id;
    u32 fb_id;

    u32 x;
    u32 y;

    u32 gamma_size;
    u32 mode_valid;
    struct drm_mode_modeinfo mode;
};

static struct drm_mode_crtc *drm_mode_get_crtc(
    struct arena *arena,
    i32 fd,
    u32 crtc_id
) {
    struct arena temp_arena = *arena;
    struct drm_mode_crtc *crtc = 0;

    crtc = alloc(&temp_arena, sizeof(*crtc));
    if (crtc == 0) {
        return 0;
    }

    crtc->crtc_id = crtc_id;
    i32 error = ioctl(
        fd,
        IOCTL_RDWR,
        IOCTL_DRM,
        DRM_IOCTL_MODE_GET_CRTC,
        sizeof(*crtc),
        (char *)crtc
    );
    if (error != 0) {
        return 0;
    }

    *arena = temp_arena;
    return crtc;
}

static i32 drm_mode_set_crtc(
    i32 fd,
    struct drm_mode_crtc *crtc,
    u32 *connectors,
    u32 connectors_len,
    u32 fb_id
) {
    crtc->set_connectors = connectors;
    crtc->connectors_len = connectors_len;
    crtc->fb_id = fb_id;
    return ioctl(
        fd,
        IOCTL_RDWR,
        IOCTL_DRM,
        DRM_IOCTL_MODE_SET_CRTC,
        sizeof(*crtc),
        (char *)crtc
    );
}

struct drm_mode_crtc_page_flip {
    u32 crtc_id;
    u32 fb_id;
    u32 flags;
    u32 reserved;
    void *user_data;
};

enum drm_mode_page_flip {
    DRM_MODE_PAGE_FLIP_EVENT = 1,
};

static i32 drm_mode_crtc_page_flip(i32 fd, u32 crtc_id, u32 fb_id) {
    struct drm_mode_crtc_page_flip flip = {
        .crtc_id = crtc_id,
        .fb_id = fb_id,
        .flags = DRM_MODE_PAGE_FLIP_EVENT,
        .user_data = (void *)0,
    };

    return ioctl(
        fd,
        IOCTL_RDWR,
        IOCTL_DRM,
        DRM_IOCTL_MODE_PAGE_FLIP,
        sizeof(flip),
        (char *)&flip
    );
}

struct drm_event {
    u32 type;
    u32 length;
};

enum drm_event_type {
    DRM_EVENT_TYPE_FLIP_COMPLETE = 2,
};

static i32 drm_mode_handle_events(i32 fd, struct arena temp_arena) {
    i32 flip_complete = 0;

    void *buffer = alloc(&temp_arena, 4096);
    i64 len = read(fd, buffer, 4096);
    if (len < 0) {
        return (i32)len;
    }

    i64 i = 0;
    while (i < len) {
        struct drm_event *e = (struct drm_event *)(void *)((char *)buffer + i);
        if (e->type == DRM_EVENT_TYPE_FLIP_COMPLETE) {
            flip_complete = 1;
        }
        i += e->length;
    }

    return flip_complete;
}

enum color {
    COLOR_BLUE = 0x0000ff,
    COLOR_GRAY = 0xededed,
};

struct game_state {
    i32 x;
    i32 y;
    i32 vx;
    i32 vy;
    i32 nvx;
    i32 nvy;
    i32 nnvx;
    i32 nnvy;
    i32 dead;
    i64 steps;
    i64 timestep;
    char board[90 * 90];
};

static void clear_game(struct game_state *state) {
    state->x = 15;
    state->y = 45;
    state->vx = 1;
    state->vy = 0;
    state->nvx = state->vx;
    state->nvy = state->vy;
    state->nnvx = state->vx;
    state->nnvy = state->vy;
    state->dead = 0;

    state->timestep = 66L * 1000L * 1000L;
    state->steps = 0;

    for (u64 i = 0; i < sizeof(state->board); ++i) {
        state->board[i] = 0;
    }

    state->board[state->y * 90 + state->x] = 1;
}

static void update_game(struct game_state *state) {
    state->y += state->vy;
    state->x += state->vx;
    state->vx = state->nvx;
    state->vy = state->nvy;
    state->nvx = state->nnvx;
    state->nvy = state->nnvy;

    if (
        state->board[state->y * 90 + state->x] != 0 ||
        state->x > 89 ||
        state->x < 0 ||
        state->y > 89 ||
        state->y < 0
    ) {
        state->dead = 1;
    } else {
        state->board[state->y * 90 + state->x] = 1;
    }

    state->steps += 1;
    if (state->steps >= (1000L * 1000L * 1000L) / state->timestep) {
        if (state->timestep > 16L * 1000L * 1000L) {
            state->timestep -= 2L * 1000L * 1000L;
        }
        state->steps = 0;
    }
}

static void draw_game(
    struct drm_mode_dumb_buffer *buf,
    struct game_state *state,
    u32 x,
    u32 y,
    u32 scale
) {
    for (u32 i = 0; i < 90; ++i) {
        for (u32 yoff = 0; yoff < scale; ++yoff) {
            u32 cy = cy = y + i * scale + yoff;
            for (u32 j = 0; j < 90; ++j) {
                for (u32 xoff = 0; xoff < scale; ++xoff) {
                    u32 cx = x + j * scale + xoff;
                    u32 pixel_index = cy * buf->stride + cx;
                    if (state->board[i * 90 + j] == 0) {
                        buf->map[pixel_index] = (u32)COLOR_GRAY;
                    } else {
                        buf->map[pixel_index] = (u32)COLOR_BLUE;
                    }
                }
            }
        }
    }
}

static void draw_partial(
    struct drm_mode_dumb_buffer *buf,
    struct game_state *state,
    u32 x,
    u32 y,
    u32 scale,
    u32 partial
) {
    if (state->y == 0 && state->vy < 0) {
        return;
    }
    if (state->y == 89 && state->vy > 0) {
        return;
    }
    if (state->x == 0 && state->vx < 0) {
        return;
    }
    if (state->x == 89 && state->vx > 0) {
        return;
    }
    for (u32 yoff = 0; yoff < scale; ++yoff) {
        if (state->vy > 0 && yoff >= partial) {
            continue;
        }
        if (state->vy < 0 && yoff < scale - partial) {
            continue;
        }
        u32 cy = cy = y + (u32)(state->y + state->vy) * scale + yoff;
        for (u32 xoff = 0; xoff < scale; ++xoff) {
            if (state->vx > 0 && xoff >= partial) {
                continue;
            }
            if (state->vx < 0 && xoff < scale - partial) {
                continue;
            }
            u32 cx = x + (u32)(state->x + state->vx) * scale + xoff;
            u32 pixel_index = cy * buf->stride + cx;
            buf->map[pixel_index] = (u32)COLOR_BLUE;
        }
    }
}

enum main_error {
    MAIN_ERROR_NONE = 0,
    MAIN_ERROR_MMAP,
    MAIN_ERROR_OPEN_CARD0,
    MAIN_ERROR_DRM_GET_RESOURCES,
    MAIN_ERROR_DRM_FIND_CONNECTOR,
    MAIN_ERROR_DRM_GET_ENCODER,
    MAIN_ERROR_DRM_CREATE_DUMB_BUFFER,
    MAIN_ERROR_DRM_GET_CRTC,
    MAIN_ERROR_DRM_SET_CRTC,
    MAIN_ERROR_DRM_HANDLE_EVENTS,
    MAIN_ERROR_DRM_PAGE_FLIP,
    MAIN_ERROR_OPEN_KEYBOARD,
    MAIN_ERROR_READ_KEYBOARD,
    MAIN_ERROR_CLOCK_GETTIME,
    MAIN_ERROR_POLL,
};

i32 main(i32 argc, char **argv) {
    (void)argc;
    (void)argv;
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

    i32 card_fd = open("/dev/dri/card0", O_RDWR, 0);
    if (card_fd < 0) {
        return MAIN_ERROR_OPEN_CARD0;
    }

    struct drm_mode_resources *res = drm_mode_get_resources(&arena, card_fd);
    if (res == 0) {
        return MAIN_ERROR_DRM_GET_RESOURCES;
    }

    u32 conn_index;
    struct drm_mode_connector *conn = 0;
    for (conn_index = 0; conn_index < res->connectors_len; ++conn_index) {
        conn = drm_mode_get_connector(
            &arena,
            card_fd,
            res->connectors[conn_index]
        );
        if (conn == 0) {
            continue;
        }
        if (conn->connection == DRM_MODE_CONNECTED && conn->modes_len != 0) {
            break;
        }
    }
    if (conn_index == res->connectors_len || conn == 0) {
        return MAIN_ERROR_DRM_FIND_CONNECTOR;
    }

    struct drm_mode_encoder *enc = drm_mode_get_encoder(
        &arena,
        card_fd,
        conn->encoder_id
    );
    if (enc == 0) {
        return MAIN_ERROR_DRM_GET_ENCODER;
    }

    u32 buf_index = 0;
    struct drm_mode_dumb_buffer *bufs[2];
    bufs[0] = drm_mode_create_dumb_buffer(
        &arena,
        card_fd,
        conn->modes[0].hdisplay,
        conn->modes[0].vdisplay
    );
    bufs[1] = drm_mode_create_dumb_buffer(
        &arena,
        card_fd,
        conn->modes[0].hdisplay,
        conn->modes[0].vdisplay
    );
    if (bufs[0] == 0 || bufs[1] == 0) {
        return MAIN_ERROR_DRM_CREATE_DUMB_BUFFER;
    }

    struct drm_mode_crtc *crtc = drm_mode_get_crtc(
        &arena,
        card_fd,
        enc->crtc_id
    );
    if (crtc == 0) {
        return MAIN_ERROR_DRM_GET_CRTC;
    }

    crtc->mode = conn->modes[0];

    i32 error = drm_mode_set_crtc(
        card_fd,
        crtc,
        &conn->connector_id,
        1,
        bufs[buf_index]->fb_id
    );
    if (error != 0) {
        return MAIN_ERROR_DRM_SET_CRTC;
    }
    buf_index ^= 1;

    error = drm_mode_set_crtc(
        card_fd,
        crtc,
        &conn->connector_id,
        1,
        bufs[buf_index]->fb_id
    );
    if (error != 0) {
        return MAIN_ERROR_DRM_SET_CRTC;
    }
    buf_index ^= 1;

    error = drm_mode_crtc_page_flip(
        card_fd,
        crtc->crtc_id,
        bufs[buf_index]->fb_id
    );
    if (error != 0) {
        return MAIN_ERROR_DRM_PAGE_FLIP;
    }
    buf_index ^= 1;

    i64 elapsed = 0;
    struct timespec last, now;
    error = clock_gettime(CLOCK_MONOTONIC, &last);
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
    struct pollfd pollfds[32 + 1];
    for (i32 i = 0; i < keyboards_len; ++i) {
        pollfds[i].fd = keyboards[i];
        pollfds[i].events = POLLIN;
    }
    pollfds[keyboards_len].fd = card_fd;
    pollfds[keyboards_len].events = POLLIN;

    u32 width = bufs[0]->width;
    u32 height = bufs[0]->height;
    u32 square_len = (height > width) ? width : height;
    u32 scale = square_len / 90;
    u32 board_size = square_len - (square_len % 90);
    u32 board_x = (width / 2) - (board_size / 2);
    u32 board_y = (height / 2) - (board_size / 2);

    struct game_state game_state;
    clear_game(&game_state);

    while (1) {
        error = clock_gettime(CLOCK_MONOTONIC, &now);
        if (error != 0) {
            return MAIN_ERROR_CLOCK_GETTIME;
        }
        elapsed += time_since_ns(&now, &last);
        last = now;

        poll(pollfds, keyboards_len + 1, 0);
        for (i32 i = 0; i < keyboards_len; ++i) {
            if (pollfds[i].revents == 0) {
                continue;
            }

            i32 keyboard_fd = pollfds[i].fd;

            i64 len = read(
                keyboard_fd,
                (char *)keyboard_events,
                sizeof(keyboard_events)
            );
            if (len < 0) {
                return MAIN_ERROR_READ_KEYBOARD;
            }

            for (i32 j = 0; j < len / (i64)sizeof(*keyboard_events); ++j) {
                struct input_event *keyboard_event = &keyboard_events[j];
                if (keyboard_event->type != 1 || keyboard_event->value != 1) {
                    continue;
                }

                switch (keyboard_event->code) {
                    case KEY_ESC:
                        return MAIN_ERROR_NONE;
                    case KEY_A:
                        if (
                            game_state.nvx == game_state.vx &&
                            game_state.nvy == game_state.vy
                        ) {
                            if (game_state.vx <= 0) {
                                game_state.nvx = -1;
                                game_state.nvy = 0;
                                game_state.nnvx = -1;
                                game_state.nnvy = 0;
                            }
                        } else {
                            if (game_state.nvx <= 0) {
                                game_state.nnvx = -1;
                                game_state.nnvy = 0;
                            }
                        }
                        break;
                    case KEY_D:
                        if (
                            game_state.nvx == game_state.vx &&
                            game_state.nvy == game_state.vy
                        ) {
                            if (game_state.vx >= 0) {
                                game_state.nvx = 1;
                                game_state.nvy = 0;
                                game_state.nnvx = 1;
                                game_state.nnvy = 0;
                            }
                        } else {
                            if (game_state.nvx >= 0) {
                                game_state.nnvx = 1;
                                game_state.nnvy = 0;
                            }
                        }
                        break;
                    case KEY_W:
                        if (
                            game_state.nvx == game_state.vx &&
                            game_state.nvy == game_state.vy
                        ) {
                            if (game_state.vy <= 0) {
                                game_state.nvx = 0;
                                game_state.nvy = -1;
                                game_state.nnvx = 0;
                                game_state.nnvy = -1;
                            }
                        } else {
                            if (game_state.nvy <= 0) {
                                game_state.nnvx = 0;
                                game_state.nnvy = -1;
                            }
                        }
                        break;
                    case KEY_S:
                        if (
                            game_state.nvx == game_state.vx &&
                            game_state.nvy == game_state.vy
                        ) {
                            if (game_state.vy >= 0) {
                                game_state.nvx = 0;
                                game_state.nvy = 1;
                                game_state.nnvx = 0;
                                game_state.nnvy = 1;
                            }
                        } else {
                            if (game_state.nvy >= 0) {
                                game_state.nnvx = 0;
                                game_state.nnvy = 1;
                            }
                        }
                        break;
                    default:
                        continue;
                }
            }
        }

        while (elapsed >= game_state.timestep) {
            elapsed -= game_state.timestep;
            update_game(&game_state);

            if (game_state.dead) {
                clear_game(&game_state);
            }
        }

        if (pollfds[keyboards_len].revents != 0) {
            i32 result = drm_mode_handle_events(card_fd, arena);
            if (result < 0) {
                return MAIN_ERROR_DRM_HANDLE_EVENTS;
            }
            if (result > 0) {
                draw_game(bufs[buf_index], &game_state, board_x, board_y, scale);
                draw_partial(
                    bufs[buf_index],
                    &game_state,
                    board_x,
                    board_y,
                    scale,
                    (u32)((elapsed * (i64)scale) / game_state.timestep)
                );
                error = drm_mode_crtc_page_flip(
                    card_fd,
                    crtc->crtc_id,
                    bufs[buf_index]->fb_id
                );
                if (error != 0) {
                    return MAIN_ERROR_DRM_PAGE_FLIP;
                }
                buf_index ^= 1;
            }
        }
    }

    return MAIN_ERROR_NONE;
}

void _cstart(i32 argc, char **argv) {
    exit(main(argc, argv));
}

