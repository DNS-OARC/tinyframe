#include <tinyframe/tinyframe.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char content_type[] = "tinyframe.test";

int main(int argc, const char* argv[])
{
    if (argc < 3) {
        return 1;
    }

    if (argv[1][0] == 'w') {
        FILE* fp = fopen(argv[2], "w");
        if (!fp) {
            return 1;
        }

        struct tinyframe_writer writer = TINYFRAME_WRITER_INITIALIZER;

        uint8_t out[sizeof(content_type) + (TINYFRAME_CONTROL_FRAME_LENGTH_MAX * 3)];
        size_t  len   = sizeof(out);
        size_t  wrote = 0;

        if (tinyframe_write_control_start(&writer, &out[wrote], len, content_type, sizeof(content_type) - 1) != tinyframe_ok) {
            return 1;
        }
        wrote += writer.bytes_wrote;
        len -= writer.bytes_wrote;

        if (tinyframe_write_frame(&writer, &out[wrote], len, (uint8_t*)content_type, sizeof(content_type)) != tinyframe_ok) {
            return 1;
        }
        wrote += writer.bytes_wrote;
        len -= writer.bytes_wrote;

        if (tinyframe_write_control_stop(&writer, &out[wrote], len) != tinyframe_ok) {
            return 1;
        }
        wrote += writer.bytes_wrote;
        len -= writer.bytes_wrote;

        if (fwrite(out, 1, wrote, fp) != wrote) {
            return 1;
        }

        fclose(fp);
        return 0;
    } else if (argv[1][0] == 'r') {
        FILE* fp = fopen(argv[2], "r");
        if (!fp) {
            return 1;
        }

        long content_length;

        if (fseek(fp, 0, SEEK_END)) {
            return 1;
        }
        content_length = ftell(fp);
        if (fseek(fp, 0, SEEK_SET)) {
            return 1;
        }

        uint8_t* content = malloc(content_length);
        if (!content) {
            return 1;
        }

        if (fread(content, 1, content_length, fp) != content_length) {
            return 1;
        }

        fclose(fp);

        struct tinyframe_reader reader   = TINYFRAME_READER_INITIALIZER;
        uint8_t*                decoding = content;
        size_t                  left     = content_length;

        if (tinyframe_read(&reader, decoding, left) != tinyframe_have_control
            || reader.control.type != TINYFRAME_CONTROL_START) {
            return 1;
        }
        decoding += reader.bytes_read;
        left -= reader.bytes_read;

        if (tinyframe_read(&reader, decoding, left) != tinyframe_have_control_field
            || reader.control_field.type != TINYFRAME_CONTROL_FIELD_CONTENT_TYPE
            || strncmp(content_type, (const char*)reader.control_field.data, reader.control_field.length)) {
            return 1;
        }
        decoding += reader.bytes_read;
        left -= reader.bytes_read;

        while (1) {
            switch (tinyframe_read(&reader, decoding, left)) {
            case tinyframe_have_frame: {
                if (strncmp(content_type, (char*)reader.frame.data, reader.frame.length)) {
                    return 1;
                }
                break;
            }

            case tinyframe_stopped:
            case tinyframe_finished:
                return 0;

            default:
                return 1;
            }

            decoding += reader.bytes_read;
            left -= reader.bytes_read;
        }
        return 0;
    }
    return 1;
}
