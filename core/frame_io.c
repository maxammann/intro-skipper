#include "frame_io.h"

void save_frame(AVPicture *pFrame, int width, int height, char *name) {
    FILE *pFile;
    int y;

    // Open file
    pFile = fopen(name, "wb");
    if (pFile == NULL)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for (y = 0; y < height; y++)
        fwrite((pFrame->data[0] + y * pFrame->linesize[0]), 1, (size_t) (width * 3), pFile);

    // Close file
    fclose(pFile);
}

void save_frame_data(AVPicture *pFrame, uint16_t width, uint16_t height, char *name) {
    int y;

    // Open file

    FILE *pFile = fopen(name, "wb");
    if (pFile == NULL)
        return;

    fwrite(&width, sizeof(uint16_t), 1, pFile);
    fwrite(&height, sizeof(uint16_t), 1, pFile);

    //fprintf(pFile, "%d%d", width, height);

    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, (size_t) (width * 3), pFile);

    // Close file
    fclose(pFile);
}

AVFrame *read_frame_data(char *file) {

    FILE *pFile;
    long lSize;
    uint8_t *buffer;
    AVFrame *frame = av_frame_alloc();

    // Open file
    pFile = fopen(file, "r+");
    if (pFile == NULL)
        return NULL;

    uint16_t width, height;

    fread(&width, sizeof(uint16_t), 1, pFile);
    fread(&height, sizeof(uint16_t), 1, pFile);

    lSize = width * height * 3;

    buffer = calloc(1, (size_t) (lSize + 1));
    if (!buffer) fclose(pFile), fputs("memory alloc fails", stderr), exit(1);

    if (1 != fread(buffer, (size_t) lSize, 1, pFile))
        fclose(pFile), free(buffer), fputs("entire read fails", stderr), exit(1);

    // Close file
    fclose(pFile);


    frame->width = width;
    frame->height = height;

    avpicture_fill((AVPicture *) frame, buffer, AV_PIX_FMT_RGB24, width, height);

    return frame;
}
