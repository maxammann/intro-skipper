#pragma once

#include <string.h>
#include <jansson.h>
#include <libavformat/avformat.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 28, 1)

#include <libavutil/frame.h>

#endif
#pragma clang diagnostic pop

char *to_json(char *file, int64_t timestamp, int64_t start, int64_t end);
