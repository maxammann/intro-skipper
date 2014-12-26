#include "analyse.h"

void rgb(AVFrame *frame, AVFrame *pFrameRGB, AnalyseContext *context) {
    AVCodecContext *dec_ctx = context->dec_ctx;

    avpicture_fill((AVPicture *) pFrameRGB, context->buffer, AV_PIX_FMT_RGB24, context->dec_ctx->width, context->dec_ctx->height);

    sws_scale(
            context->sws_ctx,
            (uint8_t const *const *) frame->data,
            frame->linesize,
            0,
            context->dec_ctx->height,
            pFrameRGB->data,
            pFrameRGB->linesize
    );
}

void grayscale(AVFrame *frame, AVFrame *pFrameRGB, AnalyseContext *context) {
    rgb(frame, pFrameRGB, context);
    AVCodecContext *dec_ctx = context->dec_ctx;
    twobitgrayscale((AVPicture *) pFrameRGB, (AVPicture *) pFrameRGB, dec_ctx->width, dec_ctx->height);
}


AVFrame *nothing(AnalyseContext *context, AVFrame *frame) {
    return NULL;
}

AVFrame *fuzzyMatchFrame(AnalyseContext *context, AVFrame *frame) {
    AVCodecContext *dec_ctx = context->dec_ctx;

    AVFrame *pFrameRGB = av_frame_alloc();

    grayscale(frame, pFrameRGB, context);

    int result = compare((AVPicture *) compare_frame, compare_frame->width, compare_frame->height, (AVPicture *) pFrameRGB, dec_ctx->width, dec_ctx->height, limit);

    if (result) {
        pFrameRGB->pkt_dts = frame->pkt_dts;
        return pFrameRGB;
    }

    av_frame_free(&pFrameRGB);

    return NULL;
}
