#include "frame_util.h"

bool compare(AVPicture *data1, int width1, int height1, AVPicture *data2, int width2, int height2, float limit) {
    int width, height;
    float scale1x = 1, scale1y = 1, scale2x = 1, scale2y = 1;

    if (width1 > width2) {
        scale1x = (float) width2 / (float) width1;

        width = width2;
    } else {
        scale2x = (float) width1 / (float) width2;

        width = width2;
    }

    if (height1 > height2) {
        scale1y = (float) height2 / (float) height1;

        height = height2;
    } else {
        scale2y = (float) height1 / (float) height2;

        height = height1;
    }

    uint16_t pwidth = (uint16_t) (width * 3);
    uint16_t acceptable = (uint16_t) (limit * height * pwidth);

    uint8_t *row1;
    uint8_t *row2;

    int fails = 0;

    for (int y = 0; y < height; y++) {
        row1 = data1->data[0] + (int) (y * scale1y) * data1->linesize[0];
        row2 = data2->data[0] + (int) (y * scale2y) * data2->linesize[0];

        for (int x = 0; x < pwidth; x += 3) {
            uint8_t a = row1[(int) (x * scale1x)];
            uint8_t b = row2[(int) (x * scale2x)];

            if (a != b) {
                fails++;


                if (fails > acceptable) {
                    return false;
                }
            }
        }
    }

    return true;
}

void twobitgrayscale(AVPicture *source, AVPicture *target, int width, int height) {
    int pwidth = width * 3;

    for (int y = 0; y < height; y++) {

        uint8_t *row = source->data[0] + y * source->linesize[0];
        uint8_t *target_row = target->data[0] + y * target->linesize[0];

        for (int x = 0; x < pwidth; x += 3) {
            uint8_t r = (uint8_t) (.2989 * row[x]);
            uint8_t g = (uint8_t) (.5870 * row[x + 1]);
            uint8_t b = (uint8_t) (.1140 * row[x + 2]);

            uint8_t c = r + g + b;

            if (c < UINT8_MAX / 2) {
                c = UINT8_MAX;
            } else {
                c = 0;
            }

            target_row[x] = c;
            target_row[x + 1] = c;
            target_row[x + 2] = c;
        }
    }
}
