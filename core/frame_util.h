#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <libavformat/avformat.h>

void twobitgrayscale(AVPicture *source, AVPicture *target, int width, int height);

bool compare(AVPicture *data1, int width1, int height1, AVPicture *data2, int width2, int height2, float limit);
