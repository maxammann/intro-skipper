#include <string.h>
#include "output.h"


Strings *new_strings(char **strings, unsigned int amount) {
    Strings *ret = malloc(sizeof(Strings));
    ret->strings = strings;
    ret->amount = amount;
    return ret;
}

//unsigned int provide_string(unsigned char **in, int *flush, int *success, Strings *strings) {
//    if (strings->success) {
//        *success = FAILED;
//        return FINISHED;
//    }
//
//    *in = (unsigned char *) strings->strings;
//
//    *flush = 1;
//    *success = CONTINUE;
//    strings->success = 1;
//
//    return strings->size;
//}

int def_strings(Strings *strings, FILE *dest, int level) {
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char out[OUT_CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit (&strm, level);
    if (ret != Z_OK)
        return ret;


    for (int i = 0; i < strings->amount; i++) {
        char *string = strings->strings[i];
        size_t length = strlen(string);
        string[length] = '\n';


        strm.avail_in = (uInt) ((length + 1) * sizeof(char));
        strm.next_in = string;
	flush = (i == strings->amount - 1) ? Z_FINISH : Z_NO_FLUSH;

        /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
        do {
            strm.avail_out = OUT_CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush); /* no bad return value */

            assert (ret != Z_STREAM_ERROR); /* state not clobbered */

            have = OUT_CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        assert (strm.avail_in == 0);  /* all input will be used */
    }

    assert (ret == Z_STREAM_END);     /* stream will be complete */

    /* clean up and return */
    deflateEnd(&strm);
    return Z_OK;
}
