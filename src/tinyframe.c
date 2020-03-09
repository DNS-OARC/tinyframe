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

#include "config.h"

#include "tinyframe/tinyframe.h"

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
#include <assert.h>
#ifdef TINYFRAME_TRACE
#include <stdio.h>
#include <ctype.h>
static const char* printable_string(const uint8_t* data, size_t len)
{
    static char buf[512];
    size_t      r = 0, w = 0;

    while (r < len && w < sizeof(buf) - 1) {
        if (isprint(data[r])) {
            buf[w++] = data[r++];
        } else {
            if (w + 4 >= sizeof(buf) - 1) {
                break;
            }

            sprintf(&buf[w], "\\x%02x", data[r++]);
            w += 4;
        }
    }
    if (w >= sizeof(buf)) {
        buf[sizeof(buf) - 1] = 0;
    } else {
        buf[w] = 0;
    }

    return buf;
}
#define trace(x...)                                \
    fprintf(stderr, "tinyframe %s(): ", __func__); \
    fprintf(stderr, x);                            \
    fprintf(stderr, "\n");                         \
    fflush(stderr);
#else
#define trace(x...)
#define printable_string(x...)
#endif

const char* const tinyframe_state_string[] = {
    "control",
    "control_field",
    "frame",
    "done",
};

const char* const tinyframe_result_string[] = {
    "ok",
    "error",
    "have_control",
    "have_control_field",
    "have_frame",
    "stopped",
    "finished",
    "need_more",
};

static inline uint32_t _need32(const void* ptr)
{
    uint32_t v;
    memcpy(&v, ptr, sizeof(v));
    return be32toh(v);
}

static inline void _put32(void* ptr, uint32_t v)
{
    uint32_t be_v = htobe32(v);
    memcpy(ptr, &be_v, sizeof(be_v));
}

static inline enum tinyframe_result __read_control(struct tinyframe_reader* handle, const uint8_t* data, size_t len)
{
    if (len < 12) {
        trace("data len %zu < 12, need more", len);
        return tinyframe_need_more;
    }
    handle->control.length = _need32(data); // "escape"
    if (handle->control.length) {
        trace("control length !zero, error, header: %s", printable_string(data, 12));
        return tinyframe_error;
    }
    handle->control.length = _need32(data + 4); // length
    if (handle->control.length > TINYFRAME_CONTROL_FRAME_LENGTH_MAX) {
        trace("control length > max, error, header: %s", printable_string(data, 12));
        return tinyframe_error;
    }
    handle->control.type = _need32(data + 8); // type

    switch (handle->control.type) {
    case TINYFRAME_CONTROL_ACCEPT:
    case TINYFRAME_CONTROL_START:
    case TINYFRAME_CONTROL_READY:
        break;
    case TINYFRAME_CONTROL_STOP:
        handle->state      = tinyframe_done;
        handle->bytes_read = 12;
        return tinyframe_stopped;
    case TINYFRAME_CONTROL_FINISH:
        handle->state      = tinyframe_done;
        handle->bytes_read = 12;
        return tinyframe_finished;
    default:
        trace("control type %d invalid, error", handle->control.type);
        return tinyframe_error;
    }

    // "escape" and length are not included in length, if not just type
    // then we have control fields
    if (handle->control.length > 4) {
        handle->state               = tinyframe_control_field;
        handle->control_length      = handle->control.length - 4; // - type length
        handle->control_length_left = handle->control_length;
        trace("control %d len %zu, next fields", handle->control.type, handle->control_length);
    } else {
        handle->state = tinyframe_frame;
        trace("control %d len %zu, next frame", handle->control.type, handle->control_length);
    }

    handle->bytes_read = 12;
    return tinyframe_have_control;
}

enum tinyframe_result tinyframe_read(struct tinyframe_reader* handle, const uint8_t* data, size_t len)
{
    assert(handle);
    assert(data);

    switch (handle->state) {
    case tinyframe_control:
        return __read_control(handle, data, len);

    case tinyframe_control_field:
        if (len < 8) {
            trace("data len %zu < 8 for control field, need more", len);
            return tinyframe_need_more;
        }
        handle->control_field.type = _need32(data);
        switch (handle->control_field.type) {
        case TINYFRAME_CONTROL_FIELD_CONTENT_TYPE:
            break;
        default:
            trace("control field type %d invalid, error", handle->control_field.type);
            return tinyframe_error;
        }

        handle->control_field.length = _need32(data + 4);
        if (handle->control_field.length > TINYFRAME_CONTROL_FIELD_CONTENT_TYPE_LENGTH_MAX) {
            trace("control field length > max, error");
            return tinyframe_error;
        }
        if (len - 8 < handle->control_field.length) {
            trace("data len %zu < control field length, need more", len - 8);
            return tinyframe_need_more;
        }

        // did we over read control frame length?
        if (handle->control_length_left < handle->control_field.length + 8) {
            trace("control field goes beyond control, error");
            return tinyframe_error;
        }
        handle->control_length_left -= handle->control_field.length + 8;
        if (!handle->control_length_left) {
            trace("no control fields left, next frame");
            handle->state = tinyframe_frame;
        }

        handle->control_field.data = data + 8;
        handle->bytes_read         = 8 + handle->control_field.length;
        trace("control field %d, data: %s", handle->control_field.type, printable_string(handle->control_field.data, handle->control_field.length));
        return tinyframe_have_control_field;

    case tinyframe_frame:
        if (len < 4) {
            trace("data len %zu < 4 for frame, need more", len);
            return tinyframe_need_more;
        }
        handle->frame.length = _need32(data);
        if (!handle->frame.length) {
            handle->state = tinyframe_control;
            return __read_control(handle, data, len);
        }

        if (len - 4 < handle->frame.length) {
            trace("data len %zu < frame length, need more", len - 4);
            return tinyframe_need_more;
        }

        handle->frame.data = data + 4;
        handle->bytes_read = 4 + handle->frame.length;
        trace("frame data [%zu]: %s...", handle->bytes_read, printable_string(data, handle->bytes_read > 20 ? 20 : handle->bytes_read));
        return tinyframe_have_frame;

    case tinyframe_done:
        break;
    }

    return tinyframe_error;
}

enum tinyframe_result tinyframe_write_control(struct tinyframe_writer* handle, uint8_t* out, size_t len, uint32_t type, const struct tinyframe_control_field* fields, size_t num_fields)
{
    size_t   out_len = 12;
    size_t   n;
    uint8_t* outp;

    for (n = 0; n < num_fields; n++) {
        if (fields[n].length > TINYFRAME_CONTROL_FIELD_CONTENT_TYPE_LENGTH_MAX) {
            trace("field %zu length > max, error", n);
            return tinyframe_error;
        }
        out_len += 8 + fields[n].length;
    }

    if (len < out_len) {
        trace("not enought space, need more");
        return tinyframe_need_more;
    }

    _put32(out, 0); // "escape"
    _put32(out + 4, out_len - 8); // length
    // - 8 is because "escape" and length is not included in length
    _put32(out + 8, type); // type

    outp = out + 12;
    for (n = 0; n < num_fields; n++) {
        _put32(outp, fields[n].type);
        _put32(outp + 4, fields[n].length);
        if (fields[n].length) {
            memcpy(outp + 8, fields[n].data, fields[n].length);
        }
        outp += 8 + fields[n].length;
    }

    handle->bytes_wrote = out_len;
    trace("control %u data: %s", type, printable_string(out, handle->bytes_wrote));
    return tinyframe_ok;
}

enum tinyframe_result tinyframe_write_control_start(struct tinyframe_writer* handle, uint8_t* out, size_t len, const char* content_type, size_t content_type_len)
{
    if (content_type_len > TINYFRAME_CONTROL_FIELD_CONTENT_TYPE_LENGTH_MAX) {
        trace("field length > max, error");
        return tinyframe_error;
    }
    if (len < 12 + 8 + content_type_len) {
        trace("not enought space, need more");
        return tinyframe_need_more;
    }

    _put32(out, 0); // "escape"
    _put32(out + 4, 12 + 8 + content_type_len - 8); // length
    // - 8 is because "escape" and length is not included in length
    _put32(out + 8, TINYFRAME_CONTROL_START); // type
    _put32(out + 12, TINYFRAME_CONTROL_FIELD_CONTENT_TYPE); // field type
    _put32(out + 16, content_type_len); // field length
    memcpy(out + 20, content_type, content_type_len); // field data

    handle->bytes_wrote = 12 + 8 + content_type_len;
    trace("control start data: %s", printable_string(out, handle->bytes_wrote));
    return tinyframe_ok;
}

enum tinyframe_result tinyframe_write_frame(struct tinyframe_writer* handle, uint8_t* out, size_t len, const uint8_t* data, uint32_t data_len)
{
    if (len < 4 + data_len) {
        trace("not enought space, need more");
        return tinyframe_need_more;
    }

    _put32(out, data_len); // length
    memcpy(out + 4, data, data_len); // frame

    handle->bytes_wrote = 4 + data_len;
    trace("frame data: %s...", printable_string(out, handle->bytes_wrote > 20 ? 20 : handle->bytes_wrote));
    return tinyframe_ok;
}

enum tinyframe_result tinyframe_write_control_stop(struct tinyframe_writer* handle, uint8_t* out, size_t len)
{
    if (len < 12) {
        trace("not enought space, need more");
        return tinyframe_need_more;
    }

    _put32(out, 0); // "escape"
    _put32(out + 4, 12 - 8); // length
    // - 8 is because "escape" and length is not included in length
    _put32(out + 8, TINYFRAME_CONTROL_STOP); // type

    handle->bytes_wrote = 12;
    trace("control stop data: %s", printable_string(out, handle->bytes_wrote));
    return tinyframe_ok;
}

void tinyframe_set_header(uint8_t* frame, uint32_t frame_length)
{
    _put32(frame, frame_length);
}

/*

frame:
- 32 length (if zero == control frame)
- data

control frame:
- 32 bit "escape" (always zero)
- 32 bit frame length
- 32 bit control type

control field: FSTRM_CONTROL_FIELD_CONTENT_TYPE:
- 32 bit field type == FSTRM_CONTROL_FIELD_CONTENT_TYPE
- 32 bit content type length
- content type

control start frame:
- <control frame>
- <control field: FSTRM_CONTROL_FIELD_CONTENT_TYPE>

control accept & ready frame:
- <control frame>
- <control field: FSTRM_CONTROL_FIELD_CONTENT_TYPE>
  ...
  (additional control fields)

control stop & finish:
  nothing more

*/
