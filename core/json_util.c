#include "json_util.h"

char *to_json(char *file, int64_t timestamp, int64_t start, int64_t end) {
    json_t *root = json_object();

    json_object_set(root, "name", json_string(file));

    json_object_set(root, "timestamp", json_integer(timestamp));

    json_object_set(root, "start", json_integer(start));
    json_object_set(root, "end", json_integer(end));

    char *json = json_dumps(root, 0);
    json_decref(root);
    return json;
}


