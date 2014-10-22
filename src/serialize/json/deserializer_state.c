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

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <yajl/yajl_parse.h>
#include <wayland-util.h>

#include "command/command.h"
#include "serialize/json/common.h"
#include "serialize/json/deserializer_state.h"
#include "serialize/json/states.h"
#include "values/nil.h"
#include "values/bool.h"
#include "values/int.h"
#include "values/string.h"
#include "objects/string.h"

struct deserializer_state*
deserialize_state_new(
    yajl_callbacks* cbs,
    void* ctx
) {
    size_t s = sizeof(struct deserializer_state);
    struct deserializer_state* state;

    state = (struct deserializer_state*) serializer_yajl_state_new(s, cbs, ctx);
    if (!state) {
        return NULL;
    }

    state->current_state    = STATE_INIT;
    state->uid.uid          = NULL;
    state->tmp_command      = NULL;

    state->tmp_command_args.vals = NULL;
    state->tmp_command_args.num = 0;

    return state;
}

int
deserialize_state_init_cmds(
    struct deserializer_state* s,
    size_t cnt
) {
    wl_array_init(s->cmdbuf);
    wl_array_add(s->cmdbuf, cnt);

    return 0;
}

int
deserialize_state_new_arg(
    struct deserializer_state* s
) {
    size_t n = s->tmp_command_args.num;
    struct ws_argument* a = s->tmp_command_args.vals;

    a = realloc(a, sizeof(*s->tmp_command_args.vals) * (n + 1));

    if (!a) {
        return -ENOMEM;
    }

    s->tmp_command_args.vals            = a;
    s->tmp_command_args.vals[n].type    = indirect; // We need something to init
    s->tmp_command_args.vals[n].arg.pos = 0; // init to something sane

    s->tmp_command_args.num++;
    return 0;
}

int
deserialize_state_mod_last_arg_set_direct(
    struct deserializer_state* s
) {
    s->tmp_command_args.vals[s->tmp_command_args.num].type = indirect;
    s->tmp_command_args.vals[s->tmp_command_args.num].arg.val = NULL;
    return 0;
}

int
deserialize_state_mod_last_arg_set_indirect(
    struct deserializer_state* s,
    int val
) {
    s->tmp_command_args.vals[s->tmp_command_args.num].type = direct;
    s->tmp_command_args.vals[s->tmp_command_args.num].arg.pos = val;
    return -1;
}

int
deserialize_state_mod_last_arg_set_null(
    struct deserializer_state* s
) {
    s->tmp_command_args.vals[s->tmp_command_args.num].type = indirect;
    struct ws_value_nil* nil = calloc(1, sizeof(*nil));
    if (!nil) {
        return -ENOMEM;
    }
    ws_value_nil_init(nil);
    struct ws_value* nil_ = (struct ws_value*) nil; // cast helper
    s->tmp_command_args.vals[s->tmp_command_args.num].arg.val = nil_;

    return 0;
}

int
deserialize_state_mod_last_arg_set_bool(
    struct deserializer_state* s,
    bool b
) {
    s->tmp_command_args.vals[s->tmp_command_args.num].type = indirect;
    struct ws_value_bool* bl = calloc(1, sizeof(*bl));

    if (!bl) {
        return -ENOMEM;
    }

    bl->b = b;

    struct ws_value* bl_ = (struct ws_value*) bl; // cast helper
    s->tmp_command_args.vals[s->tmp_command_args.num].arg.val = bl_;

    return 0;
}

int
deserialize_state_mod_last_arg_set_int(
    struct deserializer_state* s,
    int i
) {
    s->tmp_command_args.vals[s->tmp_command_args.num].type = indirect;
    struct ws_value_int* int_ = calloc(1, sizeof(*int_));
    if (!i) {
        return -ENOMEM;
    }
    int_->i = i;

    struct ws_value* v = (struct ws_value*) int_;

    s->tmp_command_args.vals[s->tmp_command_args.num].arg.val = v;
    return 0;
}

int
deserialize_state_mod_last_arg_set_string(
    struct deserializer_state* s,
    struct ws_value_string* str
) {
    s->tmp_command_args.vals[s->tmp_command_args.num].type = indirect;
    struct ws_value* str_ = (struct ws_value*) str; // cast helper
    s->tmp_command_args.vals[s->tmp_command_args.num].arg.val = str_;
    return 0;
}

int
deserialize_state_push_cmd(
    struct deserializer_state* s
) {
    if (!s->tmp_command || s->tmp_command_args.num == 0) {
        return -EINVAL;
    }

    struct ws_statement* stat = calloc(1, sizeof(*stat));
    if (!stat) {
        return -ENOMEM;
    }

    struct ws_statement** write = wl_array_add(s->cmdbuf, 1);
    if (!write) {
        return -ENOMEM;
    }

    stat->command = s->tmp_command;
    memcpy(&stat->args, &s->tmp_command_args, sizeof(stat->args));

    write[0] = stat;

    return 0;
}