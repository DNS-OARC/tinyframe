/*
 * Author Jerry Lundström <jerry@dns-oarc.net>
 * Copyright (c) 2020, OARC, Inc.
 * All rights reserved.
 *
 * This file is part of the tinyframe library.
 *
 * tinyframe library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tinyframe library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tinyframe library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <tinyframe/tinyframe.h>

#include <assert.h>
#include <string.h>
#ifdef HAVE_ENDIAN_H
#include <endian.h>
#else
#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#else
#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif
#endif
#endif

static inline uint32_t _need32(const void* ptr)
{
    uint32_t v;
    memcpy(&v, ptr, sizeof(v));
    return be32toh(v);
}

int main(void)
{
    assert(tinyframe_frame_size(0) == 4);

    struct tinyframe_writer writer = TINYFRAME_WRITER_INITIALIZER;
    static struct tinyframe_control_field _content_type = {
        TINYFRAME_CONTROL_FIELD_CONTENT_TYPE,
        4,
        (uint8_t*)"test",
    };
    uint8_t out[64];

    // correct control
    assert(tinyframe_write_control(&writer, out, sizeof(out), TINYFRAME_CONTROL_READY, 0, 0) == tinyframe_ok);
    assert(tinyframe_write_control(&writer, out, sizeof(out), TINYFRAME_CONTROL_READY, &_content_type, 1) == tinyframe_ok);

    // wrong control field type
    _content_type.type = 0;
    assert(tinyframe_write_control(&writer, out, sizeof(out), TINYFRAME_CONTROL_READY, &_content_type, 1) == tinyframe_error);
    _content_type.type = TINYFRAME_CONTROL_FIELD_CONTENT_TYPE;

    // too large control field
    _content_type.length = TINYFRAME_CONTROL_FIELD_CONTENT_TYPE_LENGTH_MAX + 1;
    assert(tinyframe_write_control(&writer, out, sizeof(out), TINYFRAME_CONTROL_READY, &_content_type, 1) == tinyframe_error);
    _content_type.length = 4;

    // too small buffer
    assert(tinyframe_write_control(&writer, out, 1, TINYFRAME_CONTROL_READY, &_content_type, 1) == tinyframe_need_more);

    // too large control field
    assert(tinyframe_write_control_start(&writer, out, sizeof(out), "test", TINYFRAME_CONTROL_FIELD_CONTENT_TYPE_LENGTH_MAX + 1) == tinyframe_error);

    // too small buffer
    assert(tinyframe_write_control_start(&writer, out, 1, "test", 4) == tinyframe_need_more);

    // too small buffer
    assert(tinyframe_write_frame(&writer, out, 1, out, 4) == tinyframe_need_more);

    // too small buffer
    assert(tinyframe_write_control_stop(&writer, out, 1) == tinyframe_need_more);

    // correct
    tinyframe_set_header(out, 111);
    assert(_need32(out) == 111);

    return 0;
}
