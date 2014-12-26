#include "skimmer.h"

AnalyseContext *new_analyse_context(void) {
    AnalyseContext *context = (AnalyseContext *) calloc(1, sizeof(AnalyseContext));

    return context;
}

void free_analyse_context(AnalyseContext *context) {
    avcodec_close(context->dec_ctx);
    avformat_close_input(&context->fmt_ctx);
    sws_freeContext(context->sws_ctx);

    free(context);
}

int open_input_file(const char *filename, AnalyseContext *context) {
    context->path = filename;

    int ret;
    AVCodec *dec;
    if ((ret = avformat_open_input(&context->fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(context->fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    /* select the video stream */
    ret = av_find_best_stream(context->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }
    context->video_stream_index = ret;
    context->dec_ctx = context->fmt_ctx->streams[context->video_stream_index]->codec;
    av_opt_set_int(context->dec_ctx, "refcounted_frames", 1, 0);

    /* init the video decoder */
    if ((ret = avcodec_open2(context->dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

    return 0;
}

void init() {
    av_register_all();
}

int frame_rate(AnalyseContext *context) {
    return context->fmt_ctx->streams[context->video_stream_index]->avg_frame_rate.num;
}

int64_t duration(AnalyseContext *context) {
    return context->fmt_ctx->duration;
}

int64_t frames(AnalyseContext *context) {
    int64_t d = duration(context) / AV_TIME_BASE;
    AVRational framerate = context->fmt_ctx->streams[context->video_stream_index]->avg_frame_rate;
    return (framerate.num * d) / framerate.den;
}

void prepareSws(AnalyseContext *context) {
    AVCodecContext *dec_ctx = context->dec_ctx;
    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, dec_ctx->width, dec_ctx->height);
    context->buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    context->sws_ctx = sws_getContext
            (
                    dec_ctx->width,
                    dec_ctx->height,
                    dec_ctx->pix_fmt,
                    dec_ctx->width,
                    dec_ctx->height,
                    AV_PIX_FMT_RGB24,
                    SWS_BILINEAR,
                    NULL,
                    NULL,
                    NULL
            );
}

void process_file(AnalyseContext *context,
        char *file,
        AVFrame *( *process )(AnalyseContext *, AVFrame *),
        void ( *progress )(AnalyseContext *, int64_t, int64_t),
        int ( *next )(AnalyseContext *, AVFrame *, void *),
        void *next_user_data
) {
    int error;

    AVFrame *frame = av_frame_alloc();

    if (!frame) {
        perror("Could not allocate frame");
        exit(1);
    }

    AVPacket packet;
    int got_frame;

    if ((error = open_input_file(file, context)) < 0) {
        goto end;
    }

    AVCodecContext *dec_ctx = context->dec_ctx;
    AVFormatContext *fmt_ctx = context->fmt_ctx;
    int video_stream_index = context->video_stream_index;

    int64_t nFrames = frames(context);
    int64_t current = 0;

    prepareSws(context);

    while (true) {
        if ((error = av_read_frame(fmt_ctx, &packet)) < 0) {
            break;
        }

        if (packet.stream_index == video_stream_index) {
            got_frame = false;

            error = avcodec_decode_video2(dec_ctx, frame, &got_frame, &packet);

            if (error < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding video\n");
                break;
            }

            if (got_frame) {
                current++;
                context->current = current;

                progress(context, nFrames, current);

                AVFrame *pFrame = process(context, frame);
                if (pFrame != NULL) {
                    if (!next(context, pFrame, next_user_data)) {
                        goto end;
                    }
                }

                av_frame_unref(frame);
            }
        }
        av_free_packet(&packet);
    }

    end:
    av_frame_free(&frame);
    if (error < 0 && error != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str (error));
    }
}
