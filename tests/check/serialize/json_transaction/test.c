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

/**
 * @addtogroup tests "Testing"
 *
 * @{
 */

/**
 * @addtogroup tests_objects "Testing: Serializer"
 *
 * @{
 */

/**
 * @addtogroup tests_objects "Testing: Serializer: Transaction test"
 *
 * @{
 */

#include <check.h>
#include "tests.h"

#include "serialize/deserializer.h"
#include "serialize/serializer.h"
#include "serialize/json/deserializer.h"
#include "serialize/json/serializer.h"
#include "objects/message/message.h"
#include "serialize/json/keys.h"
#include "util/arithmetical.h"

static const struct {
    const char* in;
    const char* out;
} inputs[] = {
    {
        .in =   "{"
                    "\"" TYPE "\": \"" TYPE_EVENT "\","
                    "\"" EVENT_NAME "\": \"testevent\","
                    "\"" EVENT_VALUE "\": 1"
                "}",

        .out =  "{\"event\":{\"context\":1,\"name\":\"testevent\"}}"
    },
    {
        .in =   "{"
                    "\"" TYPE "\": \"" TYPE_EVENT "\","
                    "\"" EVENT_NAME "\": \"testevent\","
                    "\"" EVENT_VALUE "\": \"testvalue\""
                "}",

        .out =  "{\"event\":{\"context\":\"testvalue\",\"name\":\"testevent\"}}"
    },
    {
        .in =   "{"
                    "\"" TYPE "\": \"" TYPE_EVENT "\","
                    "\"" EVENT_NAME "\": \"testevent\","
                    "\"" EVENT_VALUE "\": null"
                "}",

        .out =  "{\"event\":{\"context\":null,\"name\":\"testevent\"}}"
    },
    {
        .in =   "{"
                    "\"" TYPE "\": \"" TYPE_EVENT "\","
                    "\"" EVENT_NAME "\": \"testevent\","
                    "\"" EVENT_VALUE "\": true"
                "}",

        .out =  "{\"event\":{\"context\":true,\"name\":\"testevent\"}}"
    },
};

/*
 *
 * Test
 *
 */
START_TEST (test_inout) {
    int i = _i;
    struct ws_deserializer* d = ws_serializer_json_deserializer_new();
    ck_assert(d);

    struct ws_serializer* s = ws_serializer_json_serializer_new();
    ck_assert(s);

    struct ws_message* message = NULL;
    const char* msg = inputs[i].in;

    ws_deserialize(d, &message, msg, strlen(msg));
    ck_assert(message);

    size_t nbuf = strlen(msg);
    char buf[nbuf];
    buf[nbuf - 1] = 0;
    ws_serialize(s, buf, nbuf, message);

    ck_assert(0 == strncmp(buf, inputs[i].out, strlen(inputs[i].out)));

    ws_deserializer_deinit(d);
    ws_serializer_deinit(s);
    free(d);
    free(s);
}
END_TEST

/*
 *
 * main()
 *
 */

static Suite*
json_serializer_transaction_suite(void)
{
    Suite* s    = suite_create("JSON (De)Serializer Suite");
    TCase* tc   = tcase_create("main case");

    suite_add_tcase(s, tc);

    tcase_add_loop_test(tc, test_inout, 0, ARYLEN(inputs));

    return s;
}

WS_TESTS_CHECK_MAIN(json_serializer_transaction_suite);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

