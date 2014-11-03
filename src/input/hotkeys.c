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

#include <linux/input.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>

#include "action/manager.h"
#include "input/hotkey_dag.h"
#include "input/hotkey_event.h"
#include "input/hotkeys.h"
#include "logger/module.h"
#include "objects/message/event.h"
#include "util/cleaner.h"

#define MAX_KEYS (16)

/**
 * Hotkey subsystem context
 */
struct {
    struct ws_hotkey_dag_node root;
    struct ws_hotkey_dag_node* state;
    uint16_t key_pressed[MAX_KEYS];
    struct ws_logger_context log;
} ws_hotkeys_ctx;

/*
 *
 * Forward declarations
 *
 */

/**
 * Cleanup the hotkeys subsystem
 */
static void
hotkeys_deinit(
    void* dummy
);

/**
 * Reset the track list
 */
static void
tracklist_reset(void);

/*
 *
 * Interface implementation
 *
 */

int
ws_hotkeys_init(void) {
    static bool is_init = false;
    if (is_init) {
        return 0;
    }
    int res;


    res = ws_hotkey_dag_init(&ws_hotkeys_ctx.root);
    if (!res) {
        goto cleanup;
    }
    ws_hotkeys_ctx.state = &ws_hotkeys_ctx.root;

    ws_cleaner_add(hotkeys_deinit, NULL);

    tracklist_reset();

    ws_hotkeys_ctx.log.prefix = "[Input/Hotkeys] ";

    is_init = true;
    return 0;

cleanup:
    return res;
}

bool
ws_hotkeys_eval(
    struct input_event* ev
) {
    // get the key code
    uint16_t code = ev->code;

    // check whether the key was pressed or released
    if (ev->value) {
        // insert the key into the track list, not caring if it's full already
        unsigned int pos = MAX_KEYS;
        while (pos--) {
            if (ws_hotkeys_ctx.key_pressed[pos] == KEY_RESERVED) {
                ws_hotkeys_ctx.key_pressed[pos] = code;
                break;
            }
        }

        // jump to the next DAG node
        ws_hotkeys_ctx.state = ws_hotkey_dag_next(ws_hotkeys_ctx.state, code);
        if (!ws_hotkeys_ctx.state) {
            // this key is not part of a keycombo
            tracklist_reset();
            ws_log(&ws_hotkeys_ctx.log, LOG_DEBUG,
                   "Keycode %d not part of key-combo", code);
            return false;
        }

        ws_log(&ws_hotkeys_ctx.log, LOG_INFO,
               "Recognizing %d as part of key-combo", code);
        return true;
    }

    // remove the key from the track list, not caring if we stored or not
    unsigned int pos = MAX_KEYS;
    bool is_empty = true;
    while (pos--) {
        if (pos == code) {
            ws_hotkeys_ctx.key_pressed[pos] = KEY_RESERVED;
        }
        is_empty &= (ws_hotkeys_ctx.key_pressed[pos] == KEY_RESERVED);
    }

    // check whether we just removed the last key
    if (!is_empty) {
        // nope. Means the input still might be a key combo
        return true;
    }

    ws_log(&ws_hotkeys_ctx.log, LOG_INFO, "All keys released");
    // we have something which may be a key combo. Let's find out what to do
    if (!ws_hotkeys_ctx.state->event) {
        // nothing!
        return false;
    }

    // construct the event to emit
    struct ws_event* event;
    event = ws_event_new(&ws_hotkeys_ctx.state->event->name, NULL);
    if (!event) {
        return false;
    }

    // emit the event
    {
        char* buf = ws_string_raw(&ws_hotkeys_ctx.state->event->name);
        ws_log(&ws_hotkeys_ctx.log, LOG_INFO, "Emitting event %s...",
               buf ? buf : "unknown event");
        free(buf);
    }

    struct ws_reply* reply;
    reply = ws_action_manager_process((struct ws_message*) event);
    if (reply) {
        ws_object_unref((struct ws_object*) reply);
    }

    return false;
}

int
ws_hotkey_add(
    struct ws_hotkey_event* event
) {
   return ws_hotkey_dag_insert(&ws_hotkeys_ctx.root, event);
}

int
ws_hotkey_remove(
    struct ws_hotkey_event* event
) {
    return ws_hotkey_dag_remove(&ws_hotkeys_ctx.root, event);
}

/*
 *
 * Internal implementation
 *
 */

static void
hotkeys_deinit(
    void* dummy
) {
    ws_hotkey_dag_deinit(&ws_hotkeys_ctx.root);
}

static void
tracklist_reset(void) {
    unsigned int pos = MAX_KEYS;
    while (pos--) {
        ws_hotkeys_ctx.key_pressed[pos] = KEY_RESERVED;
    }
}

