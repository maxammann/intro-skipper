#include <json_util.h>

#include <analyse.h>
#include <frame_io.h>

#include <argp.h>

/* Program documentation. */
static char doc[] =
        "intro-extractor -- a program to extract frame information";

/* A description of the arguments we accept. */
static char args_doc[] = "FRAME FILE";

/* The options we understand. */
static struct argp_option options[] = {
        {"verbose", 'v', 0, 0},
        {"quiet", 'q', 0, 0},
        {"output", 'o', "FILES", 0},
        {0}
};

struct arguments {
    char *file;
    char *output;
    int start_frame, silent, verbose;
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
        case ARGP_KEY_ARG:
            if (state->arg_num >= 2) {
                // Too many arguments.
                argp_usage(state);
            }

            if (state->arg_num == 0) {
                arguments->start_frame = atoi(arg);
            } else {
                arguments->file = arg;
            }

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

int lookup;

/**
* You will need to free the returned frame: av_free ( frame );
*/
AVFrame *findFrame(AnalyseContext *context, AVFrame *frame) {
    uint64_t time = (uint64_t) frame->pkt_dts;

    if (time == lookup) {
        AVFrame *pFrameRGB = av_frame_alloc();

        grayscale(frame, pFrameRGB, context);

        pFrameRGB->pkt_dts = frame->pkt_dts;
        return pFrameRGB;
    }

    return NULL;
}

int main(int argc, char **argv) {
    struct arguments arguments;

    arguments.silent = 0;
    arguments.verbose = 0;
    arguments.output = "";

    argp_parse(&argp, argc, argv, 0, 0, &arguments);


    lookup = arguments.start_frame;

    printf("Extracting %s\n", arguments.file);

    AnalyseContext *context = new_analyse_context();

    AVFrame *found; //todo
    process_file(context, arguments.file, findFrame, NULL, NULL, NULL);


    char buffer[100];
    char *source = arguments.file;

    AVCodecContext *dec_ctx = context->dec_ctx;

    // Output
    strcpy(buffer, source);
    save_frame_data((AVPicture *) found, dec_ctx->width, dec_ctx->height, strcat(buffer, ".fd"));
    strcpy(buffer, source);
    save_frame((AVPicture *) found, dec_ctx->width, dec_ctx->height, strcat(buffer, ".ppm"));
    strcpy(buffer, source);
    writeFrame(strcat(buffer, ".json"), argv[1], frame_rate(context), found);

    free_analyse_context(context);
    return 0;
}
