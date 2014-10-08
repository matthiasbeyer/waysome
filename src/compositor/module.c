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

#include "compositor/module.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-server.h>
#include <xf86drm.h>

#include "util/cleaner.h"
#include "util/wayland.h"
#include "logger/module.h"
#include "compositor/internal_context.h"
#include "background_surface.h"
#include "monitor.h"

struct ws_compositor_context ws_comp_ctx;
struct ws_logger_context log_ctx = { .prefix = "[Compositor] " };


/**
 * Find the connector associated with a crtc id
 *
 * This function iterates over all connectors, searching for a connector with
 * the id specified.
 *
 * @return the connector for a crtc or NULL, if no such connector exists.
 */
static struct ws_monitor*
find_connector_with_crtc(
    int crtc
);


/**
 * Find a suitable connector and return it's id
 *
 * This function tries to find a connector which is
 *  - free
 *  - matches the encoding
 *  - can be used to access the monitor passed to the function
 * and sets the crtc for a monitor
 *
 * @returns 0 or a negative error value (-ENOENT)
 */
static int
find_crtc(
    drmModeRes* res,
    drmModeConnector* conn,
    struct ws_monitor* connector
);

/**
 * Find all connectors on the given framebuffer device
 *
 * The connectors found are added to the given compositor context directly.
 *
 * @return 0 on success, a negative error code otherwise (-ENOENT)
 */
static int
populate_connectors(void);

/**
 * Create framebuffers for all connected connectors
 *
 * @return 0 on success, a negative error code otherwise (-ENOENT)
 */
static int
populate_framebuffers(
    void* dummy,
    void const* mon
);

/**
 * Deinitialise the compositor
 *
 * This function deinitialises the compositor.
 * It does not take care of inter-module dependency nor does it check whether
 * it was called before, unlike ws_compositor_init().
 *
 * @warning call this function only once
 */
static void
ws_compositor_deinit(
    void* dummy
);


/*
 *
 * Interface implementation
 *
 */

int
ws_compositor_init(void) {
    static bool is_init = false;
    if (is_init) {
        return 0;
    }

    ws_log(&log_ctx, "Initing monitor set");
    ws_set_init(&ws_comp_ctx.monitors);

    ws_log(&log_ctx, "Starting initialization of the Compositor.");

    ws_cleaner_add(ws_compositor_deinit, NULL);
    int retval;

    ws_comp_ctx.fb = ws_framebuffer_device_new("/dev/dri/card0");

    retval = populate_connectors();
    if (retval < 0) {
        return retval;
    }

    retval = ws_set_select(&ws_comp_ctx.monitors, NULL, NULL,
            populate_framebuffers, NULL);
    if (retval < 0) {
        return retval;
    }

    //!< @todo: Port to buffer code once it is implemented
    // struct ws_monitor* it = ws_comp_ctx.conns;
    // struct ws_image_buffer* duck = ws_background_service_load_image("duck.png");
    // while (it && duck->buffer) {
    //     if (!it->connected) {
    //         it = it->next;
    //         continue;
    //     }
    //     ws_log(&log_ctx, "Copying into monitor with name: %s", it->mode.name);
    //     for (int i = 0; i < duck->height; ++i) {
    //         memcpy(it->map + (it->stride * i), (char*)duck->buffer +
    //                 (duck->stride * i), duck->stride);
    //     }
    //     it = it->next;
    // }

    // initialize wayland specific stuff
    struct wl_display* display = ws_wayland_acquire_display();
    if (!display) {
        return -1;
    }

    // try to initialize the shared memory subsystem
    if (wl_display_init_shm(display) != 0) {
        goto cleanup_display;
    }

    ws_wayland_release_display();


    is_init = true;
    return 0;

    // cleanup wayland display

cleanup_display:
    ws_wayland_release_display();
    return -1;
}


/*
 *
 * Internal implementation
 *
 */


static void
ws_compositor_deinit(
    void* dummy
) {

    if (ws_comp_ctx.fb->fd >= 0) {
        close(ws_comp_ctx.fb->fd);
    }

    //!< @todo: free all of the framebuffers

    //!< @todo: prelimary: free the preloaded PNG
}

static struct ws_monitor*
find_connector_with_crtc(
        int crtc
) {
    struct ws_monitor dummy;
    dummy.crtc = crtc;
    dummy.fb_dev = ws_comp_ctx.fb;
    return (struct ws_monitor*)ws_set_get(&ws_comp_ctx.monitors,
            (struct ws_object*)&dummy);
}

static int
find_crtc(
    drmModeRes* res,
    drmModeConnector* conn,
    struct ws_monitor* connector
) {
    drmModeEncoder* enc;
    int32_t crtc;

    // If we do have an encoder, we check that noone else uses this crtc
    // if (enc) {
    //     if (enc->crtc_id) {
    //         ws_log(&log_ctx, "There seems to be a crtc on here.");
    //         crtc = enc->crtc_id;

    //         if (find_connector_with_crtc(crtc) != NULL) {
    //             ws_log(&log_ctx, "There was a crtc! Setting it.");
    //             drmModeFreeEncoder(enc);
    //             connector->crtc = crtc;
    //             return 0;
    //         }
    //     }

    //     drmModeFreeEncoder(enc);
    // }
    // There is no encoder+crtc pair! We go through /all/ the encoders now
    for (int i = 0; i < conn->count_encoders; ++i) {
        enc = drmModeGetEncoder(ws_comp_ctx.fb->fd, conn->encoders[i]);

        if (!enc) {
            ws_log(&log_ctx, "Could not get Encoder.");
            continue;
        }

        ws_log(&log_ctx, "Found %d crtcs", res->count_crtcs);
        for( int j = 0; j < res->count_crtcs; ++j) {
            // Check if this encoding supports with all the crtcs
            if(!(enc->possible_crtcs & (1 << j))) {
                continue;
            }

            // Check noone else uses it!
            crtc = res->crtcs[j];

            // Looks like we found one! Return!
            if (find_connector_with_crtc(crtc) == NULL) {
                drmModeFreeEncoder(enc);
                ws_log(&log_ctx, "Found a CRTC! Saving");
                connector->crtc = crtc;
                return 0;
            }
        }

        drmModeFreeEncoder(enc);
    }

    ws_log(&log_ctx, "Could not find suitable Encoder for crtc with dim: %dx%d.",
            connector->width, connector->height);
    return -ENOENT;
}

static int
populate_connectors(void) {
    drmModeRes* res;
    drmModeConnector* conn;
    // struct ws_monitor* connector = ws_comp_ctx.conns;

    res = drmModeGetResources(ws_comp_ctx.fb->fd);
    if (!res) {
        ws_log(&log_ctx, "Could not get Resources for: %s.",
                ws_comp_ctx.fb->path);
        return -ENOENT;
    }

    // Let's go through all connectors (outputs)
    int i = res->count_connectors;
    ws_log(&log_ctx, "Found a max of %d connectors.", i);
    while(i--) {
        conn = drmModeGetConnector(ws_comp_ctx.fb->fd, res->connectors[i]);
        if (!conn) {
            ws_log(&log_ctx, "Could not get connector for: %s",
                    ws_comp_ctx.fb->path);
            goto insert;
        }
        struct ws_monitor* new_monitor = ws_monitor_new();
        new_monitor->conn = conn->connector_id;
        new_monitor->fb_dev = ws_comp_ctx.fb;

        if (conn->connection != DRM_MODE_CONNECTED) {
            ws_log(&log_ctx, "Found unused connector %d", i);
            new_monitor->connected = 0;
            goto insert;
        }

        if (conn->count_modes == 0) {
            ws_log(&log_ctx, "No valid modes for Connector %d.",
                    conn->connector_id);
            new_monitor->connected = 0;
            goto insert;
        }

        ws_log(&log_ctx, "Found a valid connector with %d modes.",
                conn->count_modes);

        //!< @todo: Do not just take the biggest mode available
        memcpy(&new_monitor->mode, &conn->modes[0],
                sizeof(new_monitor->mode));

        new_monitor->width = conn->modes[0].hdisplay;
        new_monitor->height = conn->modes[0].vdisplay;

        ws_log(&log_ctx, "Found a valid connector with %dx%d dimensions.",
                new_monitor->width, new_monitor->height);

        if (find_crtc(res, conn, new_monitor) < 0) {
            ws_log(&log_ctx, "No valid crtcs found");
            new_monitor->connected = 0;
            goto insert;
        }
        ws_log(&log_ctx, "Found a valid crtc with id %d", new_monitor->crtc);
        new_monitor->connected = 1;

insert:
        ws_set_insert(&ws_comp_ctx.monitors, (struct ws_object*)new_monitor);
    }
    return 0;
}

static int
populate_framebuffers(
    void* dummy,
    void const* mon
) {
    struct ws_monitor* monitor = (struct ws_monitor*) mon;
    struct drm_mode_create_dumb creq; //Create Request
    struct drm_mode_destroy_dumb dreq; //Delete Request
    struct drm_mode_map_dumb mreq; //Memory Request

    if (!monitor->connected) {
        ws_log(&log_ctx, "Did not create FB for monitor %d.", monitor->crtc);
        return 0;
    }
    memset(&creq, 0, sizeof(creq));
    creq.width = monitor->width;
    creq.height = monitor->height;
    creq.bpp = 32;
    int ret = drmIoctl(ws_comp_ctx.fb->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) {
        ws_log(&log_ctx, "Could not create DUMB BUFFER");
        return 0;
    }

    monitor->stride = creq.pitch;
    monitor->size = creq.size;
    monitor->handle = creq.handle;

    ret = drmModeAddFB(ws_comp_ctx.fb->fd, monitor->width, monitor->height, 24,
            32, monitor->stride, monitor->handle, &monitor->fb);

    if (ret) {
        ws_log(&log_ctx, "Could not add FB of size: %dx%d.",
                creq.width, creq.height);
        goto err_destroy;
    }

    memset(&mreq, 0, sizeof(mreq));
    mreq.handle = monitor->handle;
    ret = drmIoctl(ws_comp_ctx.fb->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (ret) {
        ws_log(&log_ctx, "Could not allocate enough memory for FB.");
        goto err_fb;
    }

    monitor->map = mmap(0, monitor->size, PROT_READ | PROT_WRITE, MAP_SHARED,
            ws_comp_ctx.fb->fd, mreq.offset);

    if (monitor->map == MAP_FAILED) {
        ws_log(&log_ctx, "Could not MMAP FB");
        goto err_fb;
    }

    memset(monitor->map, 0, monitor->size);

    monitor->saved_crtc = drmModeGetCrtc(ws_comp_ctx.fb->fd, monitor->crtc);
    ret = drmModeSetCrtc(ws_comp_ctx.fb->fd, monitor->crtc, monitor->fb, 0, 0,
            &monitor->conn, 1, &monitor->mode);
    if (ret) {
        ws_log(&log_ctx, "Could not set the CRTC for monitor %d.",
                monitor->crtc);
        goto err_fb;
    }

    ws_log(&log_ctx, "Succesfully created Framebuffer");

    return 0;
err_fb:
    drmModeRmFB(ws_comp_ctx.fb->fd, monitor->fb);
err_destroy:
    memset(&dreq, 0, sizeof(dreq));
    dreq.handle = monitor->handle;
    drmIoctl(ws_comp_ctx.fb->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);

    return 0;
}
