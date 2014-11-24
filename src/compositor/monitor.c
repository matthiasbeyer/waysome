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
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-server.h>
#include <wayland-server-protocol.h>
#include <xf86drm.h>

#include "compositor/buffer/image.h"
#include "compositor/framebuffer_device.h"
#include "compositor/internal_context.h"
#include "compositor/monitor.h"
#include "logger/module.h"
#include "objects/object.h"
#include "util/wayland.h"
#include "values/union.h"
#include "values/value_type.h"

/*
 *
 *  Forward declarations
 *
 */

static bool
monitor_deinit(
    struct ws_object* self
);

static size_t
monitor_hash(
    struct ws_object* self
);

static int
monitor_cmp(
    struct ws_object const* obj1,
    struct ws_object const* obj2
);

static void
bind_output(
    struct wl_client* client,
    void* data,
    uint32_t version,
    uint32_t id
);

static int
publish_modes(
    void* _data,
    void const* _mode
);

/**
 * Callback command function for setting background
 *
 * Takes one argument:
 *  - The path to the image (PNG) as string
 *
 * @memberof ws_monitor
 */
static int
cmd_func_set_background(
    union ws_value_union* stack // The stack to use
);

/*
 *
 * type variable
 *
 */

/**
 * Callback function table for commands
 */
static const struct ws_object_function FUNCTIONS[] = {
    { .name = "setbgpng",         .func = cmd_func_set_background },
    { .name = NULL,             .func = NULL } // iteration stopper
};

ws_object_type_id WS_OBJECT_TYPE_ID_MONITOR = {
    .supertype = &WS_OBJECT_TYPE_ID_OBJECT,
    .typestr = "ws_monitor",
    .deinit_callback = monitor_deinit,
    .hash_callback = monitor_hash,
    .cmp_callback = monitor_cmp,

    .attribute_table = NULL,
    .function_table = FUNCTIONS,
};

/*
 *
 * Interface Implementation
 *
 */

struct ws_monitor*
ws_monitor_new(
    void
) {
    struct ws_monitor* tmp = calloc(1, sizeof(*tmp));
    ws_object_init((struct ws_object*) tmp);
    tmp->obj.id = &WS_OBJECT_TYPE_ID_MONITOR;
    tmp->obj.settings |= WS_OBJECT_HEAPALLOCED;

    // initialize members
    if (ws_set_init(&tmp->surfaces) < 0) {
        goto cleanup_alloc;
    }

    if (ws_set_init(&tmp->modes) < 0) {
        goto cleanup_alloc;
    }


    return tmp;

cleanup_alloc:
    free(tmp);
    return tmp;
}

struct ws_set*
ws_monitor_surfaces(
    struct ws_monitor* self
) {
    return &self->surfaces;
}

void
ws_monitor_publish(
    struct ws_monitor* self
) {
    struct wl_display* display = ws_wayland_acquire_display();
    if (!display) {
        return;
    }

    self->global = wl_global_create(display, &wl_output_interface, 2,
                    self, &bind_output);

    ws_wayland_release_display();

}

int
ws_monitor_set_bg_from_path(
    struct ws_monitor* self,
    char const* path
) {
    if (!self || !path) {
        return -EINVAL;
    }

    struct ws_image_buffer* img = ws_image_buffer_from_png(path);
    if (!img) {
        return -ENOENT;
    }

    ws_buffer_blit(&self->buffer->obj.obj, &img->obj);
    ws_object_unref(&img->obj.obj);
    return 0;
}

int
ws_monitor_set_bg_from_buffer(
    struct ws_monitor* self, //!< The monitor object
    struct ws_image_buffer* buff //!< The buffer to use
) {
    if (!self || !buff) {
        return -EINVAL;
    }

    ws_buffer_blit(&self->buffer->obj.obj, &buff->obj);
    return 0;
}

void
ws_monitor_populate_fb(
    struct ws_monitor* self
) {

    if (!self->connected) {
        ws_log(&log_ctx, LOG_DEBUG,
                "Did not create FB for self %d.", self->crtc);
        return;
    }

    if (!self->current_mode) {
        ws_log(&log_ctx, LOG_ERR,
                "No mode set, can't create Framebuffer");
        return;
    }

    self->buffer = ws_frame_buffer_new(
            self->fb_dev,
            self->current_mode->mode.hdisplay,
            self->current_mode->mode.vdisplay
    );
    if (!self->buffer) {
        ws_log(&log_ctx, LOG_CRIT, "Could not create Framebuffer");
    }
    self->saved_crtc = drmModeGetCrtc(ws_comp_ctx.fb->fd, self->crtc);

    int ret = drmModeSetCrtc(ws_comp_ctx.fb->fd, self->crtc, self->buffer->fb,
            0, 0, &self->conn, 1, &self->current_mode->mode);
    if (ret) {
        ws_log(&log_ctx, LOG_ERR, "Could not set the CRTC for self %d.",
                self->crtc);
    }
    return;
}

void
ws_monitor_set_mode_with_id(
    struct ws_monitor* self,
    int id
) {
    struct ws_monitor_mode mode;
    memset(&mode, 0, sizeof(mode));
    mode.obj.id = &WS_OBJECT_TYPE_ID_MONITOR_MODE;
    mode.id = id;
    self->current_mode =
        (struct ws_monitor_mode*) ws_set_get(
                &self->modes,
                (struct ws_object*)&mode);

    if (!self->current_mode) {
        return;
    }

    if (!self->resource) {
        ws_log(&log_ctx, LOG_DEBUG, "Did not publish mode.");
        return;
    }
    ws_log(&log_ctx, LOG_DEBUG, "Published a mode.");

    // Let's tell wayland that this is the current mode!
    wl_output_send_mode((struct wl_resource*) self->resource,
            WL_OUTPUT_MODE_CURRENT, self->current_mode->mode.hdisplay,
            self->current_mode->mode.vdisplay,
            // The buffer and wayland differ on which unit to use
            self->current_mode->mode.vrefresh * 1000);
}

struct ws_monitor_mode*
ws_monitor_copy_mode(
    struct ws_monitor* self,
    struct _drmModeModeInfo const* src
) {
    struct ws_monitor_mode* mode = ws_monitor_mode_new();
    memcpy(&mode->mode, src, sizeof(*src));
    mode->id = self->mode_count++;
    ws_set_insert(&self->modes, (struct ws_object*) mode);
    return mode;
}

struct ws_monitor_mode*
ws_monitor_add_mode(
    struct ws_monitor* self,
    int width,
    int height
) {
    //!< We exit rather than letting the program finish as it will hang the
    // graphic card
    ws_log(&log_ctx, LOG_CRIT, "Looks like we're in a doozy! This is undefined"
            " behaviour and shouldn't be called. 'ws_monitor_add_mode'");
    exit(1);
    struct ws_monitor_mode* mode = ws_monitor_mode_new();
    if (!mode) {
        ws_log(&log_ctx, LOG_ERR, "Could not create mode.");
        return NULL;
    }
    mode->id = self->mode_count++;
    mode->mode.hdisplay = width;
    mode->mode.vdisplay = height;
    mode->mode.clock = 155;
    //!< @todo: Add more information like vsync etc...
    ws_set_insert(&self->modes, (struct ws_object*) mode);
    return mode;
}

/*
 *
 * Internal implementation
 *
 */

static void
bind_output(
    struct wl_client* client,
    void* data,
    uint32_t version,
    uint32_t id
) {

    struct ws_monitor* monitor = data;

    if (version >= 2) {
        // We only support from 2 and ongoing, so let's say that!
        version = 2;
    }

    monitor->resource = wl_resource_create(client, &wl_output_interface,
            version, id);

    if (!monitor->resource) {
        ws_log(&log_ctx, LOG_ERR, "Wayland couldn't create object");
        return;
    }

    // We don't set an implementation, instead we just set the data
    wl_resource_set_implementation(monitor->resource, NULL, data, NULL);

    // The origin of this monitor, 0,0 in this case
    wl_output_send_geometry(monitor->resource, 0, 0, monitor->phys_width,
            // 0 is the subpixel type, and the two strings are monitor infos
            monitor->phys_height, 0, "unknown", "unknown",
            WL_OUTPUT_TRANSFORM_NORMAL);
    // We publish all the modes we have right now through wayland
    ws_set_select(&monitor->modes, NULL, NULL, publish_modes,
                  monitor->resource);

    // We tell wayland that this output is done!
    wl_output_send_done(monitor->resource);
}


static int
publish_modes(
    void* _data,
    void const* _mode
) {
    struct ws_monitor_mode* mode = (struct ws_monitor_mode*) _mode;
    // The 0 here means that this isn't a preferred nor the current display
    wl_output_send_mode((struct wl_resource*) _data, 0,
            mode->mode.hdisplay, mode->mode.vdisplay,
            // The buffer and wayland differ on which unit to use
            mode->mode.vrefresh * 1000);
    return 0;
}

static bool
monitor_deinit(
    struct ws_object* obj
) {
    struct ws_monitor* self = (struct ws_monitor*) obj;
    if (self->connected) {
        drmModeSetCrtc(self->fb_dev->fd,
                self->saved_crtc->crtc_id,
                self->saved_crtc->buffer_id,
                self->saved_crtc->x,
                self->saved_crtc->y,
                &self->conn,
                1,
                &self->saved_crtc->mode);
    }

    ws_object_deinit((struct ws_object*) &self->surfaces);
    return true;
}

static size_t
monitor_hash(
    struct ws_object* obj
) {
    struct ws_monitor* self = (struct ws_monitor*) obj;
    return SIZE_MAX / (self->crtc * self->fb_dev->fd + 1);
}

static int
monitor_cmp(
    struct ws_object const* obj1,
    struct ws_object const* obj2
) {
    struct ws_monitor* mon1 = (struct ws_monitor*) obj1;
    struct ws_monitor* mon2 = (struct ws_monitor*) obj2;

    if (mon1->id != mon2->id) {
        return (mon1->id > mon2->id) - (mon1->id < mon2->id);
    }

    if (mon1->fb_dev->fd != mon2->fb_dev->fd) {
        return (mon1->fb_dev->fd > mon2->fb_dev->fd) -
            (mon1->fb_dev->fd < mon2->fb_dev->fd);
    }
    return 0;
}

static int
cmd_func_set_background(
    union ws_value_union* stack // The stack to use
) {
    union ws_value_union* retval = stack;
    struct ws_monitor* mon;

    if (ws_value_get_type(&stack->value) != WS_VALUE_TYPE_OBJECT_ID) {
        return -EINVAL;
    }

    mon = (struct ws_monitor*) ws_value_object_id_get(&stack->object_id);

    if (((struct ws_object*) mon)->id != &WS_OBJECT_TYPE_ID_MONITOR) {
        return -EINVAL;
    }

    stack += 2;

    if (ws_value_get_type(&stack->value) != WS_VALUE_TYPE_STRING) {
        return -EINVAL;
    }

    struct ws_string* str   = ws_value_string_get(&stack->string);
    const char* path        = ws_string_raw(str);

    int res = ws_monitor_set_bg_from_path(mon, path);

    int r;
    r = ws_value_union_reinit(retval, WS_VALUE_TYPE_BOOL);
    if (!r) {
        return r;
    }

    return ws_value_bool_set(&retval->bool_, res == 0);
}

