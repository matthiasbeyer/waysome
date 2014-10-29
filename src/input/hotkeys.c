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

#include "input/hotkeys.h"
#include "input/hotkey_dag.h"
#include "util/cleaner.h"

#define MAX_KEYS (16)

/**
 * Hotkey subsystem context
 */
struct {
    struct ws_hotkey_dag_node root;
    struct ws_hotkey_dag_node* state;
    uint16_t key_pressed[MAX_KEYS];
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

    unsigned int pos = MAX_KEYS;
    while (pos--) {
        ws_hotkeys_ctx.key_pressed[pos] = KEY_RESERVED;
    }

    is_init = true;
    return 0;

cleanup:
    return res;
}

bool
ws_hotkeys_eval(
    struct input_event* ev
) {
    //!< @todo implement
    return false;
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

