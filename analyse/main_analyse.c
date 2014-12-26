#include <json_util.h>
#include <argp.h>

#include <frame_io.h>

#include "analyse.h"

/* Program documentation. */
static char doc[] =
        "intro-skipper -- a program to search video files for intros";

/* A description of the arguments we accept. */
static char args_doc[] = "FIND FILE2";

/* The options we understand. */
static struct argp_option options[] = {
        {"verbose", 'v', 0, 0},
        {"quiet", 'q', 0, 0},
        {"output", 'o', "FILES", 0},
        {"limit", 'l', "LIMIT", 0},
        {0}
};

struct arguments {
    char *files[2];
    char *output;
    int start_frame, silent, verbose;
    float limit;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'q':
            arguments->silent = 1;
            break;
        case 'v':
            arguments->verbose = 1;
            break;
        case 'o':
            arguments->output = arg;
            break;
        case 'l':
            arguments->limit = atof(arg);
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 2) {
                // Too many arguments.
                argp_usage(state);
            }


            arguments->files[state->arg_num] = arg;

            break;
        case ARGP_KEY_END:
            if (state->arg_num < 2) {
                // Not enough arguments.
                argp_usage(state);
            }
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
    struct arguments arguments;

    arguments.silent = 0;
    arguments.verbose = 0;
    arguments.output = "";
    arguments.limit = 0.05;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    char buffer[100];
    char **files = arguments.files;

    compare_frame = read_frame_data(files[0]);
    limit = arguments.limit;

    printf("Analysing %s\n", files[1]);
    AnalyseContext *context = new_analyse_context();
    AVFrame *pFrame; //todo

    process_file(context, files[1], fuzzyMatchFrame, NULL, NULL, NULL);

    // Prepend path
    strcpy(buffer, arguments.output);
    strcat(buffer, files[1]);
    char target[100];
    strcpy(target, buffer);

    AVCodecContext *dec_ctx = context->dec_ctx;

    //Output
    strcpy(buffer, target);
    save_frame_data((AVPicture *) pFrame, dec_ctx->width, dec_ctx->height, strcat(buffer, ".fd"));
    strcpy(buffer, target);
    save_frame((AVPicture *) pFrame, dec_ctx->width, dec_ctx->height, strcat(buffer, ".ppm"));
    strcpy(buffer, target);
    writeFrame(strcat(buffer, ".json"), files[1], frame_rate(context), pFrame);

    av_frame_free(&pFrame);
    av_frame_free(&compare_frame);

    free_analyse_context(context);
    return 0;
}
