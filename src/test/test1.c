#include <tinyframe/tinyframe.h>

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void
print_string(const void* data, size_t len)
{
    uint8_t* str = (uint8_t*)data;
    putc('"', stdout);
    while (len-- != 0) {
        unsigned c = *(str++);
        if (isprint(c)) {
            if (c == '"')
                puts("\\\"");
            else
                putc(c, stdout);
        } else {
            printf("\\x%02x", c);
        }
    }
    putc('"', stdout);
}

int main(int argc, const char* argv[])
{
    if (argc < 3) {
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        return 2;
    }

    int rbuf_len = atoi(argv[2]);

    struct tinyframe_reader h = TINYFRAME_READER_INITIALIZER;

    size_t  s = 0, r;
    uint8_t buf[4096], rbuf[rbuf_len];
    while ((r = fread(rbuf, 1, sizeof(rbuf), fp)) > 0) {
        printf("read %zu\n", r);

        if (s + r > sizeof(buf)) {
            printf("overflow\n");
            break;
        }
        memcpy(&buf[s], rbuf, r);
        s += r;

        int r = 1;
        while (r) {
            switch (tinyframe_read(&h, buf, s)) {
            case tinyframe_have_control:
                printf("control type %" PRIu32 " len %" PRIu32 "\n", h.control.type, h.control.length);
                break;
            case tinyframe_have_control_field:
                printf("control_field type %" PRIu32 " len %" PRIu32 " data: ", h.control_field.type, h.control_field.length);
                print_string(h.control_field.data, h.control_field.length);
                printf("\n");
                break;
            case tinyframe_have_frame:
                printf("frame len %" PRIu32 " data: ", h.frame.length);
                print_string(h.frame.data, h.frame.length);
                printf("\n");
                break;
            case tinyframe_need_more:
                printf("need more\n");
                r = 0;
                break;
            case tinyframe_error:
                printf("error\n");
                fclose(fp);
                return 2;
            case tinyframe_stopped:
                printf("stopped\n");
                fclose(fp);
                return 0;
            case tinyframe_finished:
                printf("finished\n");
                fclose(fp);
                return 0;
            default:
                printf("unexpected return code\n");
                fclose(fp);
                return 3;
            }

            if (r && h.bytes_read && h.bytes_read <= s) {
                s -= h.bytes_read;
                if (s) {
                    memmove(buf, &buf[h.bytes_read], s);
                }
            }
        }
    }

    fclose(fp);
    return 0;
}
