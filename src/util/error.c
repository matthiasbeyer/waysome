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

#include <stdlib.h>
#include <string.h>

#include "values/string.h"
#include "util/arithmetical.h"
#include "util/condition.h"
#include "util/error.h"

char*
ws_errno_tostr(
    int errnr
) {
    return strdup(strerror(ABS(errnr)));
}

struct ws_value_string*
ws_errno_to_value_string(
    int errno
) {
    const char* errstr = ws_errno_tostr(errno);
    if (!errstr) {
        return NULL;
    }

    struct ws_value_string* res = ws_value_string_new();
    if (unlikely(!res)) {
        goto out;
    }

    struct ws_string* s = ws_value_string_get(res);
    if (unlikely(!s)) {
        goto cleanup_values;
    }

    if (ws_string_set_from_raw(s, errstr) != 0) {
        goto cleanup_values;
    }

    goto out;

cleanup_values:
    ws_value_deinit((struct ws_value*) res);
    res = NULL;

out:
    return res;
}

