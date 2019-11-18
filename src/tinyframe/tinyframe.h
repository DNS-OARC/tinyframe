/*
 * Author Jerry Lundström <jerry@dns-oarc.net>
 * Copyright (c) 2019, OARC, Inc.
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

#include <unistd.h>
#include <stdint.h>

#ifndef __tinyframe_h_tinyframe
#define __tinyframe_h_tinyframe 1

#define TINYFRAME_CONTROL_FRAME_LENGTH_MAX 512
#define TINYFRAME_CONTROL_FIELD_CONTENT_TYPE_LENGTH_MAX 256

#define TINYFRAME_CONTROL_ACCEPT 0x01
#define TINYFRAME_CONTROL_START 0x02
#define TINYFRAME_CONTROL_STOP 0x03
#define TINYFRAME_CONTROL_READY 0x04
#define TINYFRAME_CONTROL_FINISH 0x05

#define TINYFRAME_CONTROL_FIELD_CONTENT_TYPE 0x01

struct tinyframe_control {
    uint32_t length;
    uint32_t type;
};

#define TINYFRAME_CONTROL_INITIALIZER \
    {                                 \
        .length = 0,                  \
        .type   = 0,                  \
    }

struct tinyframe_control_field {
    uint32_t       length;
    uint32_t       type;
    const uint8_t* data;
};

#define TINYFRAME_CONTROL_FIELD_INITIALIZER \
    {                                       \
        .length = 0,                        \
        .type   = 0,                        \
        .data   = 0,                        \
    }

struct tinyframe {
    uint32_t       length;
    const uint8_t* data;
};

#define TINYFRAME_INITIALIZER \
    {                         \
        .length = 0,          \
        .data   = 0,          \
    }

enum tinyframe_state {
    tinyframe_control,
    tinyframe_control_field,
    tinyframe_frame,
};

struct tinyframe_reader {
    enum tinyframe_state state;

    size_t control_length;

    struct tinyframe_control       control;
    struct tinyframe_control_field control_field;
    struct tinyframe               frame;

    size_t bytes_read;
};

#define TINYFRAME_READER_INITIALIZER                           \
    {                                                          \
        .state          = tinyframe_control,                   \
        .control_length = 0,                                   \
        .control        = TINYFRAME_CONTROL_INITIALIZER,       \
        .control_field  = TINYFRAME_CONTROL_FIELD_INITIALIZER, \
        .frame          = TINYFRAME_INITIALIZER,               \
        .bytes_read     = 0,                                   \
    }

struct tinyframe_writer {
    size_t bytes_wrote;
};

#define TINYFRAME_WRITER_INITIALIZER \
    {                                \
        .bytes_wrote = 0,            \
    }

enum tinyframe_result {
    tinyframe_ok,
    tinyframe_error,
    tinyframe_have_control,
    tinyframe_have_control_field,
    tinyframe_have_frame,
    tinyframe_stopped,
    tinyframe_finished,
    tinyframe_need_more,
};

enum tinyframe_result tinyframe_read(struct tinyframe_reader*, const uint8_t*, size_t);

enum tinyframe_result tinyframe_write_control_start(struct tinyframe_writer*, uint8_t*, size_t, const char*, size_t);
enum tinyframe_result tinyframe_write_frame(struct tinyframe_writer*, uint8_t*, size_t, const uint8_t*, size_t);
enum tinyframe_result tinyframe_write_control_stop(struct tinyframe_writer*, uint8_t*, size_t);

#endif
