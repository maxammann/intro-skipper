#pragma once

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <stdbool.h>

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 28, 1)
#define av_frame_alloc()        avcodec_alloc_frame()
#define av_frame_unref(frame)
#define av_frame_free(frame)    av_free(frame)
#define av_err2str(error)       error
#endif

struct AnalyseContext_ {
    const char *path;
    AVFormatContext *fmt_ctx;
    AVCodecContext *dec_ctx;
    int video_stream_index;

    struct SwsContext *sws_ctx;
    uint8_t *buffer;

    int64_t current;
};

typedef struct AnalyseContext_ AnalyseContext;

AnalyseContext *new_analyse_context();

void free_analyse_context(AnalyseContext *context);

int frame_rate(AnalyseContext *context);

int64_t frames(AnalyseContext *context);

int64_t duration(AnalyseContext *context);

int open_input_file(const char *filename, AnalyseContext *context);

void init();

void prepareSws(AnalyseContext *context);

void process_file(AnalyseContext *context,
        char *file,
        AVFrame *( *process )(AnalyseContext *, AVFrame *),
        void ( *progress )(AnalyseContext *, int64_t, int64_t),
        int ( *next )(AnalyseContext *, AVFrame *, void *),
        void *next_user_data
);
