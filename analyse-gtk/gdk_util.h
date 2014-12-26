#pragma once

#include <libavformat/avformat.h>
#include <gtk/gtk.h>
#include <stdint.h>

void copy_frame_pixbuf(GdkPixbuf *pixbuf, AVFrame *frame, int width, int height);
