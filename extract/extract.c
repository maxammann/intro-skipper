#include "extract.h"

ExtractContext *new_extract_context() {
    ExtractContext *context = (ExtractContext *) calloc(1, sizeof(ExtractContext));

    return context;
}

AVFrame *extract(ExtractContext *context,
        int64_t time
) {
    int error;

    AVCodecContext *dec_ctx = context->dec_ctx;
    AVFormatContext *fmt_ctx = context->fmt_ctx;

    AVPacket packet;
    int got_frame;

    int64_t seek_target = av_rescale_q(time, AV_TIME_BASE_Q, fmt_ctx->streams[context->video_stream_index]->time_base);

    if ((error = av_seek_frame(fmt_ctx, context->video_stream_index, seek_target, 0)) < 0) {
        return NULL;
    }

    avcodec_flush_buffers(dec_ctx);

    AVFrame *frame = av_frame_alloc();

    while (true) {
        if (av_read_frame(fmt_ctx, &packet) < 0) {
            av_frame_free(&frame);
            break;
        }

        if (packet.stream_index == context->video_stream_index) {
            got_frame = false;

            avcodec_decode_video2(dec_ctx, frame, &got_frame, &packet);
            if (got_frame) {
                break;
            }

            av_frame_unref(frame);
        }

        av_free_packet(&packet);
    }


    av_free_packet(&packet);
    return frame;
}

