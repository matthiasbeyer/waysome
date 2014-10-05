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

#ifndef __WS_OBJECTS_SET_H__
#define __WS_OBJECTS_SET_H__

#include "objects/object.h"

/**
 * ws_set type definition
 *
 * The ws_set type implementation operates on libreset internally.
 * The `struct ws_set` type is only used for `ws_object` objects, so nobody has
 * to care about the set configuration. Therefor, this is hold internally to the
 * set interface.
 */
struct ws_set {
    struct ws_object obj; //!< superclass

    struct r_set_cfg set_config; //!< Configuration for libreset sets
    struct r_set* set; //!< Actual set implementation
};

/**
 * Variable which holds type information about the ws_set type
 */
extern ws_object_type_id WS_OBJECT_TYPE_ID_SET;

/**
 * Predicate function type
 */
typedef int (*ws_set_predf)(void const*, void*);

/**
 * Processor function type
 */
typedef int (*ws_set_procf)(void*, void const*);

/**
 * Initialize a set object
 *
 * @memberof ws_set
 *
 * @return zero on success, else negative error code from errno.h:
 *          -EINVAL - NULL passed
 *          -ENOMEM - Set initialization failed
 */
int
ws_set_init(
    struct ws_set* self //!< The set object
);

/**
 * Get a new, initialized set object
 *
 * @memberof ws_set
 *
 * @return new set object or NULL on failure
 */
struct ws_set*
ws_set_new(void);

/**
 * Insert an object into the ste
 *
 * @memberof ws_set
 *
 * @return zero on success, else negative error value from errno.h
 */
int
ws_set_insert(
    struct ws_set* self, //!< The set
    struct ws_object* obj //!< The object to insert
);

/**
 * Remove an object from the set by a unique identification
 *
 * @memberof ws_set
 *
 * @return zero on success else negative error value from errno.h
 */
int
ws_set_remove(
    struct ws_set* self //!< The set
    /* object-UUID */
);

/**
 * Get an object from a set
 *
 * @memberof ws_set
 *
 * @return the ws_object object or NULL on failure
 */
struct ws_object*
ws_set_get(
    struct ws_set const* self //!< The set
    /* object-UUID */
);

/**
 * Create an union from two set objects
 *
 * @memberof ws_set
 *
 * @return zero on success, else negative error number from errno.h
 */
int
ws_set_union(
    struct ws_set* dest, //!< The destination set
    struct ws_set const* src_a, //!< The first source set
    struct ws_set const* src_b //!< The second source set
);

/**
 * Create an intersection from two sets
 *
 * @memberof ws_set
 *
 * @return zero on success, else negative error number from errno.h
 */
int
ws_set_intersection(
    struct ws_set* dest, //!< The destination set
    struct ws_set const* src_a, //!< The first source set
    struct ws_set const* src_b //!< The second source set
);

/**
 * Create the symmetric difference between two sets
 *
 * @memberof ws_set
 *
 * @return zero on success, else negative error number from errno.h
 */
int
ws_set_xor(
    struct ws_set* dest, //!< The destination set
    struct ws_set const* src_a, //!< The first source set
    struct ws_set const* src_b //!< The second source set
);

/**
 * Check if one set is a subset of another
 *
 * @memberof ws_set
 *
 * @return true if `other` is subset of `self`, else false
 */
bool
ws_set_is_subset(
    struct ws_set const* self, //!< The set
    struct ws_set const* other //!< The other set
);

/**
 * Check if two sets are equal
 *
 * @memberof ws_set
 *
 * @return true if `self` and `other` are equal, else false
 */
bool
ws_set_equal(
    struct ws_set const* self, //!< The set
    struct ws_set const* other //!< The set to check equality for
);

/**
 * Get the cardinality of a set
 *
 * @memberof ws_set
 *
 * @return the cardinality of `self`
 */
size_t
ws_set_cardinality(
    struct ws_set const* self //!< The set
);

/**
 * Execute a processor function for each element of a set
 *
 * @memberof ws_set
 *
 * @note `ws_set_select` can be used to execute an operation on a subset of the
 * set, selecting elements for another set or doing other crazy things for
 * elements of a set.
 *
 * @return zero on success, else negative error code from errno.h
 */
int
ws_set_select(
    struct ws_set const* self, //!< The set
    ws_set_predf pred, //!< Predicate function
    void* pred_etc, //!< Additional parameter for the predicate function
    ws_set_procf proc, //!< Processor function
    void* proc_etc //!< Additional parameter for the processor function
);

#endif // __WS_OBJECTS_SET_H__
