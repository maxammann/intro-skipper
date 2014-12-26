#pragma once

#include <skimmer.h>

struct ExtractContext_ {
    char *path;
    AVFormatContext *fmt_ctx;
    AVCodecContext *dec_ctx;
    int video_stream_index;

    struct SwsContext *sws_ctx;
    uint8_t *buffer;
};

typedef struct ExtractContext_ ExtractContext;

ExtractContext *new_extract_context();

AVFrame *extract(ExtractContext *context, int64_t time);
