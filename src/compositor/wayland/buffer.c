/*
 * waysome - wayland based window manager
 *
 * Copyright in alphabetical order:
 *
 * Copyright (C) 2014-2015 Julian Ganz
 * Copyright (C) 2014-2015 Manuel Messner
 * Copyright (C) 2014-2015 Marcel Müller
 * Copyright (C) 2014-2015 Matthias Beyer
 * Copyright (C) 2014-2015 Nadja Sommerfeld
 *
 * This file is part of waysome.
 *
 * waysome is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * waysome is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with waysome. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <malloc.h>
#include <wayland-server.h>

#include "compositor/wayland/buffer.h"
#include "logger/module.h"

/*
 *
 * Forward declarations
 *
 */

/**
 * Get the buffer's contents
 *
 * @return the buffer's contents
 */
static void*
get_data(
    struct ws_buffer const* self
);

/**
 * Get the buffer's width
 *
 * @return width of the buffer's contents
 */
static int32_t
get_width(
    struct ws_buffer const* self
);

/**
 * Get the buffer's height
 *
 * @return height of the buffer's contents
 */
static int32_t
get_height(
    struct ws_buffer const* self
);

/**
 * Get the buffer's stride
 *
 * @return height of the buffer's contents
 */
static int32_t
get_stride(
    struct ws_buffer const* self
);

/**
 * Get the buffer's format
 *
 * @return height of the buffer's contents
 */
static uint32_t
get_format(
    struct ws_buffer const* self
);

/**
 * Get the buffer's bpp
 *
 * @return bpp
 */
static uint32_t
get_bpp(
    struct ws_buffer const* self
);

/**
 * Begin a transaction
 */
static void
begin_access(
    struct ws_buffer* self
);

/**
 * End a transaction
 */
static void
end_access(
    struct ws_buffer* self
);


/*
 *
 * Internal constant
 *
 */

static struct ws_logger_context log_ctx = { .prefix = "[Compositor/Surface] " };

/**
 * Buffer type
 */
static ws_buffer_type_id buffer_type = {
    .type = {
        .supertype  = (struct ws_object_type const*) &WS_OBJECT_TYPE_ID_BUFFER,
        .typestr    = "anonymous",

        .hash_callback = NULL,

        .deinit_callback = NULL,
        .cmp_callback = NULL,

        .attribute_table = NULL,
        .function_table = NULL,
    },
    .get_data = get_data,
    .get_width = get_width,
    .get_height = get_height,
    .get_stride = get_stride,
    .get_format = get_format,
    .get_bpp = get_bpp,
    .begin_access = begin_access,
    .end_access = end_access,
};

/*
 *
 * Interface implementation
 *
 */

ws_object_type_id WS_OBJECT_TYPE_ID_WAYLAND_BUFFER = {
    .supertype  = &WS_OBJECT_TYPE_ID_WAYLAND_OBJ,
    .typestr    = "ws_wayland_buffer",

    .hash_callback = NULL,
    .deinit_callback = NULL,
    .cmp_callback = NULL,

    .attribute_table = NULL,
    .function_table = NULL,
};

int
ws_wayland_buffer_init(
    struct ws_wayland_buffer* self,
    struct wl_resource* r
) {
    if (!self) {
        return -EINVAL;
    }

    ws_wayland_obj_init(&self->wl_obj, r);
    self->wl_obj.obj.id = &WS_OBJECT_TYPE_ID_WAYLAND_BUFFER;

    // initialize members
    int retval = ws_buffer_init(&self->buf);
    if (retval < 0) {
        return retval;
    }
    self->buf.obj.id = &buffer_type.type;

    return 0;
}

struct ws_wayland_buffer*
ws_wayland_buffer_new(
    struct wl_resource* r
) {
    struct ws_wayland_buffer* w = calloc(1, sizeof(*w));
    if (!w) {
        return NULL;
    }

    if (ws_wayland_buffer_init(w, r) != 0) {
        goto cleanup;
    }
    w->wl_obj.obj.settings |= WS_OBJECT_HEAPALLOCED;

    return w;

cleanup:
    free(w);
    return NULL;
}

void
ws_wayland_buffer_set_resource(
    struct ws_wayland_buffer* self,
    struct wl_resource* r
) {
    ws_wayland_obj_set_wl_resource(&self->wl_obj, r);
}

struct ws_buffer*
ws_wayland_buffer_get_buffer(
    struct ws_wayland_buffer* self
) {
    return &self->buf;
}


/*
 *
 * Internal implementation
 *
 */

static void*
get_data(
    struct ws_buffer const* self
) {
    // get the resource
    struct ws_wayland_buffer* buf = wl_container_of(self, buf, buf);
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&buf->wl_obj);

    // assume that we have a shm buffer
    struct wl_shm_buffer* shm_buffer = wl_shm_buffer_get(res);
    return wl_shm_buffer_get_data(shm_buffer);
}

static int32_t
get_width(
    struct ws_buffer const* self
) {
    // get the resource
    struct ws_wayland_buffer* buf = wl_container_of(self, buf, buf);
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&buf->wl_obj);

    if (!res) {
        return 0;
    }
    // assume that we have a shm buffer
    struct wl_shm_buffer* shm_buffer = wl_shm_buffer_get(res);
    return wl_shm_buffer_get_width(shm_buffer);
}

static int32_t
get_height(
    struct ws_buffer const* self
) {
    // get the resource
    struct ws_wayland_buffer* buf = wl_container_of(self, buf, buf);
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&buf->wl_obj);

    if (!res) {
        return 0;
    }
    // assume that we have a shm buffer
    struct wl_shm_buffer* shm_buffer = wl_shm_buffer_get(res);
    return wl_shm_buffer_get_height(shm_buffer);
}

static int32_t
get_stride(
    struct ws_buffer const* self
) {
    // get the resource
    struct ws_wayland_buffer* buf = wl_container_of(self, buf, buf);
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&buf->wl_obj);

    // assume that we have a shm buffer
    struct wl_shm_buffer* shm_buffer = wl_shm_buffer_get(res);
    return wl_shm_buffer_get_stride(shm_buffer);
}

static uint32_t
get_format(
    struct ws_buffer const* self
) {
    // get the resource
    struct ws_wayland_buffer* buf = wl_container_of(self, buf, buf);
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&buf->wl_obj);

    // assume that we have a shm buffer
    struct wl_shm_buffer* shm_buffer = wl_shm_buffer_get(res);
    return wl_shm_buffer_get_format(shm_buffer);
}

static uint32_t
get_bpp(
    struct ws_buffer const* self
) {
    // 4 Bytes per pixel (BGRA)
    return 4;
}

static void
begin_access(
    struct ws_buffer* self
) {
    // get the resource
    struct ws_wayland_buffer* buf = wl_container_of(self, buf, buf);
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&buf->wl_obj);

    // assume that we have a shm buffer
    struct wl_shm_buffer* shm_buffer = wl_shm_buffer_get(res);
    wl_shm_buffer_begin_access(shm_buffer);
}

static void
end_access(
    struct ws_buffer* self
) {
    // get the resource
    struct ws_wayland_buffer* buf = wl_container_of(self, buf, buf);
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&buf->wl_obj);

    // assume that we have a shm buffer
    struct wl_shm_buffer* shm_buffer = wl_shm_buffer_get(res);
    wl_shm_buffer_end_access(shm_buffer);
}

void
ws_wayland_buffer_release(
    struct ws_wayland_buffer* self
) {
    if (!self) {
        ws_log(&log_ctx, LOG_ERR, "We got a NULL surface");
        return;
    }
    struct wl_resource* res = ws_wayland_obj_get_wl_resource(&self->wl_obj);
    wl_buffer_send_release(res);
}

