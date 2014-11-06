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

#ifndef __WS_INPUT_HOTKEYS_H__
#define __WS_INPUT_HOTKEYS_H__

#include <stdbool.h>

#include "util/attributes.h"

// forward declaration
struct input_event;
struct ws_hotkey_event;


/**
 * Initialize hotkeys subsystem
 *
 * @return 0 on success, a negative error if an error occurred
 */
int
ws_hotkeys_init(void);

/**
 * Evaluate event
 *
 * Check whether the event is relevant to us.
 * This may trigger an event.
 *
 * @return true if the event is part of a key combo, false if it's for an app
 */
bool
ws_hotkeys_eval(
    struct input_event* ev
)
__ws_nonnull__(1)
;

#endif // __WS_INPUT_HOTKEYS_H__

