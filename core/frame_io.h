#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <libavcodec/avcodec.h>

void save_frame(AVPicture *pFrame, int width, int height, char *name);

void save_frame_data(AVPicture *pFrame, uint16_t width, uint16_t height, char *name);

AVFrame *read_frame_data(char *file);
