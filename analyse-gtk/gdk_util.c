#include "gdk_util.h"

void copy_frame_pixbuf(GdkPixbuf *pixbuf, AVFrame *frame, int width, int height) {
    int rowstride, n_channels;
    guchar *pixels, *p;

    n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    int y;

    uint16_t pwidth = width * 3;

    uint8_t *row1;
    uint8_t *row2;

    for (int y = 0; y < height; y++) {
        row1 = frame->data[0] + y * frame->linesize[0];

        for (int x = 0; x < pwidth; x += 3) {
            uint8_t r = row1[x];
            uint8_t g = row1[x + 1];
            uint8_t b = row1[x + 2];

            p = pixels + y * rowstride + (x / 3) * n_channels;
            p[0] = r;
            p[1] = g;
            p[2] = b;
        }
    }
}
