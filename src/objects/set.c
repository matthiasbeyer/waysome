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
#include <stdlib.h>

#include <libreset/hash.h>
#include <libreset/set.h>

#include "objects/object.h"

#include "objects/set.h"

/**
 * Set configuration for libreset. Not meant to be public.
 */
static const struct r_set_cfg WS_SET_CONFIGURATION= {
    .cmpf = NULL,
    .copyf = (void const* (*)(void const*)) ws_object_getref,
    .freef = (void (*)(void*)) ws_object_unref,
    .hashf = (r_hash (*)(void const*)) ws_object_hash,
};

/**
 * Deinit callback for ws_set type
 */
static bool
deinit_set(
    struct ws_object* const self
) {
    if (self) {
        return (0 == r_set_destroy(((struct ws_set*) self)->set));
    }

    return false;
}

/**
 * Type information for ws_set type
 */
ws_object_type_id WS_OBJECT_TYPE_ID_SET = {
    .supertype  = &WS_OBJECT_TYPE_ID_OBJECT,
    .typestr    = "ws_set",

    .deinit_callback = deinit_set,
};

/*
 *
 * Interface implementation
 *
 */

int
ws_set_init(
    struct ws_set* self
) {
    if (self) {
        ws_object_init(&self->obj);

        self->obj.id = &WS_OBJECT_TYPE_ID_SET;

        self->set = r_set_new(&WS_SET_CONFIGURATION);

        if (!self->set) {
            return -ENOMEM;
        }

        return 0;
    }

    return -EINVAL;
}

struct ws_set*
ws_set_new(void)
{
    struct ws_set* set = (struct ws_set*) ws_object_new(sizeof(struct ws_set));

    if (set) {
        ws_set_init(set);
    }

    return set;
}

int
ws_set_insert(
    struct ws_set* self,
    struct ws_object* obj
) {
    if (self && obj) {
        int res = r_set_insert(self->set, obj);

        if (res == 0 || res == -EEXIST) {
            return 0;
        }

        return res;
    }

    return -EINVAL;
}

int
ws_set_remove(
    struct ws_set* self
    /* object-UUID */
) {
    return 0;
}

struct ws_object*
ws_set_get(
    struct ws_set const* self
    /* object-UUID */
) {
    return NULL;
}

// int
// ws_set_union(
//     struct ws_set* dest,
//     struct ws_set const* src_a,
//     struct ws_set const* src_b
// ) {
//     if (dest && src_a && src_b) {
//         return r_set_union(dest->set, src_a->set, src_b->set);
//     }
//
//     return -EINVAL;
// }

// int
// ws_set_intersection(
//     struct ws_set* dest,
//     struct ws_set const* src_a,
//     struct ws_set const* src_b
// ) {
//     if (dest && src_a && src_b) {
//         return r_set_intersection(dest->set, src_a->set, src_b->set);
//     }
//
//     return -EINVAL;
// }

// int
// ws_set_xor(
//     struct ws_set* dest,
//     struct ws_set const* src_a,
//     struct ws_set const* src_b
// ) {
//     if (dest && src_a && src_b) {
//         return r_set_xor(dest->set, src_a->set, src_b->set);
//     }
//
//     return -EINVAL;
// }

// bool
// ws_set_is_subset(
//     struct ws_set const* self,
//     struct ws_set const* other
// ) {
//     if (self && other) {
//         return r_set_is_subset(self->set, other->set);
//     }
// 
//     return false;
// }

bool
ws_set_equal(
    struct ws_set const* self,
    struct ws_set const* other
) {
    if (self && other) {
        return r_set_equal(self->set, other->set);
    }

    return false;
}

size_t
ws_set_cardinality(
    struct ws_set const* self
) {
    if (self) {
        return r_set_cardinality(self->set);
    }

    return 0;
}

int
ws_set_select(
    struct ws_set const* self,
    ws_set_predf pred,
    void* pred_etc,
    ws_set_procf proc,
    void* proc_etc
) {
    if (self) {
        return r_set_select(self->set, pred, pred_etc, proc, proc_etc);
    }

    return -EINVAL;
}

