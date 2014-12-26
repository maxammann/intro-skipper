#pragma once

#include <skimmer.h>
#include <frame_util.h>


AVFrame *compare_frame;
float limit;


AVFrame *nothing(AnalyseContext *context, AVFrame *frame);

void rgb(AVFrame *frame, AVFrame *pFrameRGB, AnalyseContext *context);

void grayscale(AVFrame *frame, AVFrame *pFrameRGB, AnalyseContext *context);

AVFrame *fuzzyMatchFrame(AnalyseContext *context, AVFrame *frame);
