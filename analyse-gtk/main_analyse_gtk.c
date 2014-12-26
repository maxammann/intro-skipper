#include <gtk/gtk.h>
#include <regex.h>

#include <frame_io.h>
#include <analyse.h>
#include <extract.h>
#include <json_util.h>
#include <output.h>

#include "gdk_util.h"

struct AnalyseJob_ {
    char *path;
    GList *found;
    int64_t timestamp;
};

typedef struct AnalyseJob_ AnalyseJob;

GtkListStore *store;

GtkFileChooser *input_chooser;
GtkFileChooser *analyse_input_chooser;
GtkImage *result_preview, *extract_preview;
GtkEntry *filter_test_entry;
GtkEntry *filter_input_entry;
GtkEntry *start_entry;
GtkEntry *end_entry;
GtkWidget *result_tree;

int last_progress;

GHashTable *jobs;
AnalyseJob *selected_job;

enum {
    COL_FILE = 0,
    COL_PROGRESS = 1
};

AnalyseJob *find_job(const char *file) {
    return g_hash_table_lookup(jobs, file);
}

GtkTreeIter *reference_job(AnalyseJob * job) {
    GtkTreeIter iter;
    GtkTreeModel *model = GTK_TREE_MODEL (store);
    gboolean valid;

    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
        gchar *file;

        gtk_tree_model_get(model, &iter,
                COL_FILE, &file,
                -1);

        if (strcmp(job->path, file) == 0) {
            g_free(file);
            return gtk_tree_iter_copy(&iter);
        }

        g_free(file);

        valid = gtk_tree_model_iter_next(model, &iter);
    }
}

gboolean valid(char *test, const char *regex) {

    regex_t r;

    if (regcomp(&r, regex, REG_EXTENDED | REG_NOSUB) != 0) {
        return FALSE;
    }

    if (regexec(&r, test, 0, 0, 0)) {
        regfree(&r);
        return FALSE;
    } else {
        regfree(&r);
        return TRUE;
    }
}

void iterate_recursive(GFile *file, void *iterate(GFile *)) {
    GFileType type = g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL);
    if (type != G_FILE_TYPE_DIRECTORY) {
        return;
    }

    GError *error = NULL;

    GFileEnumerator *list = g_file_enumerate_children(file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, &error);

    if (error != NULL) {
        printf("%s\n", error->message);
        return;
    }

    while (1) {
        GFileInfo *info = g_file_enumerator_next_file(list, NULL, &error);

        if (info == NULL) {
            break;
        }


        GFile *child = g_file_enumerator_get_child(list, info);

        iterate(child);

        iterate_recursive(child, iterate);
    }
}

void create_result_list(GtkWidget *result) {
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (result),
            -1,
            "File",
            gtk_cell_renderer_text_new(),
            "text", COL_FILE,
            NULL);

    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (result),
            -1,
            "Progress",
            gtk_cell_renderer_progress_new(),
            "value", COL_PROGRESS,
            NULL);

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_UINT);

    GtkTreeModel *model = GTK_TREE_MODEL (store);

    gtk_tree_view_set_model(GTK_TREE_VIEW (result), model);

    g_object_unref(model);
}


void add_result(GFile *file) {
    char *path = g_file_get_path(file); //todo should be freed

    if (!valid(path, ".*avi|.*mkv|.*mp4")) {
        return;
    }

    GtkTreeIter iter;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            COL_FILE, path,
            COL_PROGRESS, 0,
            -1);

    AnalyseJob *job = calloc(1, sizeof(AnalyseJob));
    job->path = path;
    job->found = g_list_alloc();

    g_hash_table_insert(jobs, path, job);
}

void update_result_list() {
    gtk_list_store_clear(store);
    char *path = gtk_file_chooser_get_filename(input_chooser);

    GFile *file = g_file_new_for_path(path);

    iterate_recursive(file, (void *) add_result);

    gtk_widget_queue_resize(GTK_WIDGET (result_tree));
}

void update_frame_preview() {
    GList *jobs = g_list_last(selected_job->found);

    if (jobs == NULL) {
        gtk_image_clear(result_preview);
        return;
    }

    GdkPixbuf *pixbuf = jobs->data;

    if (pixbuf == NULL) {
        gtk_image_clear(result_preview);
        return;
    }

    gtk_image_set_from_pixbuf(result_preview, pixbuf);
    //g_object_unref ( pixbuf );
}

void export_jobs() {
    GHashTableIter iter;
    gpointer key;
    AnalyseJob *value;

    char *strings[g_hash_table_size(jobs)];

    g_hash_table_iter_init(&iter, jobs);
    unsigned int i = 0;
    while (g_hash_table_iter_next(&iter, &key, (gpointer *) &value)) {
        char *json = to_json(key,
                (int64_t) (((double) value->timestamp) / 25.0 * AV_TIME_BASE),
                (int64_t) atol(gtk_entry_get_text(start_entry)) * AV_TIME_BASE,
                (int64_t) atol(gtk_entry_get_text(end_entry)) * AV_TIME_BASE);
        strings[i] = json;
        i++;
    }

    FILE *file = fopen("test.bin", "wb");
    Strings *pStrings = new_strings(strings, i);
    def_strings(pStrings, file, 9);
    fclose(file);

    free(pStrings);
}

gboolean update_progress(AnalyseContext *context) {
    int progress = (int) (((double) context->current / (double) frames(context)) * 100);

    if (progress == last_progress) return FALSE;
    last_progress = progress;

    AnalyseJob *job = find_job(context->path);
    GtkTreeIter *iter = reference_job(job);

    gtk_list_store_set(store, iter,
            COL_PROGRESS, progress,
            -1);
    return FALSE;
}

void progress(AnalyseContext *context, int64_t frames, int64_t current) {
    g_main_context_invoke(NULL, (GSourceFunc) update_progress, context);
}

int64_t last_frame = 0; //todo multithreading

static int update_job(AnalyseContext *context, AVFrame *frame, AnalyseJob *job) {
    if (context->current <= last_frame + 250) {
        return FALSE;
    }

    last_frame = context->current;

    frame->width = context->dec_ctx->width;
    frame->height = context->dec_ctx->height;

    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, frame->width, frame->height);

    if (pixbuf == NULL) {
        return FALSE;
    }

    copy_frame_pixbuf(pixbuf, frame, frame->width, frame->height);

    GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, 480, 234, GDK_INTERP_NEAREST); //todo free

    job->timestamp = frame->pkt_dts;
    job->found = g_list_append(job->found, scaled);

    g_object_unref(pixbuf);
    av_frame_unref(frame);
    return FALSE;
}

static void analyse_job(char *file, AnalyseJob *job) {
    printf("Analysing %s\n", file);
    AnalyseContext *context = new_analyse_context();

    process_file(context, file, fuzzyMatchFrame, (void *) &progress, (void *) &update_job, job);
    last_frame = 0;

    free_analyse_context(context);
}

void analyse_sync(char *analyse_file) {
    compare_frame = read_frame_data(analyse_file);
    limit = 0.1;

    GtkTreeIter iter;
    GtkTreeModel *model = GTK_TREE_MODEL (store);
    gboolean valid;

    valid = gtk_tree_model_get_iter_first(model, &iter);
    while (valid) {
        gchar *file;

        gtk_tree_model_get(model, &iter,
                COL_FILE, &file,
                -1);

        AnalyseJob *job = find_job(file);

        analyse_job(job->path, job);

        g_free(file);

        valid = gtk_tree_model_iter_next(model, &iter);
    }
}

void analyse() {
    char *analyse_file = gtk_file_chooser_get_filename(analyse_input_chooser);

    pthread_t thread;

    pthread_create(&thread, NULL, (void *) &analyse_sync, analyse_file);
}

gboolean filter() {

    const char *regex = gtk_entry_get_text(filter_input_entry);

    const char *test = gtk_entry_get_text(filter_test_entry);

    if (gtk_entry_get_text_length(filter_test_entry) == 0) {
        return TRUE;
    }

}

void row_selected(GtkTreeView *tree_view,
        gpointer user_data) {
    GtkTreeIter iter;
    GtkTreePath *path;
    gtk_tree_view_get_cursor(tree_view, &path, NULL);

    if (gtk_tree_model_get_iter(GTK_TREE_MODEL (store), &iter, path)) {
        gchar *file;

        gtk_tree_model_get(GTK_TREE_MODEL (store), &iter,
                COL_FILE, &file,
                -1);

        selected_job = find_job(file);

        g_free(file);
    }

    gtk_tree_path_free(path);

    update_frame_preview();
}

ExtractContext *context;
AVFrame *pFrameRGB;

gboolean extract_skim(GtkRange *range,
        GtkScrollType scroll,
        gdouble value,
        gpointer user_data) {
    if (pFrameRGB != NULL) {
        av_free(pFrameRGB);
    }                                //06:20 -> 06:39 -> 6:53

    if (context == NULL) {
        context = new_extract_context();
        open_input_file("/media/archive/Downloads/extracted/Battlestar.Galactica.S03E14.Allein.gegen.den.Hass.German.720p.BluRay.x264-RSG/rsg-bsg-s03e14-720p.mkv", (AnalyseContext *) context);
        prepareSws((AnalyseContext *) context);

        gtk_range_set_adjustment(range, gtk_adjustment_new(0, 0, context->fmt_ctx->duration / AV_TIME_BASE / 2, 0.1, 0, 0));
    }

    int64_t time = (int64_t) (value * (gdouble) AV_TIME_BASE);
    AVFrame *frame = extract(context, time);
    pFrameRGB = av_frame_alloc();

    grayscale(frame, pFrameRGB, (AnalyseContext *) context);

    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, context->dec_ctx->width, context->dec_ctx->height);

    copy_frame_pixbuf(pixbuf, pFrameRGB, context->dec_ctx->width, context->dec_ctx->height);

    gtk_image_set_from_pixbuf(extract_preview, pixbuf);

    g_object_unref(pixbuf);
    av_free(frame);

    return FALSE;
}

gboolean save_extract() {
    if (pFrameRGB == NULL) {
        return FALSE;
    }

    save_frame_data((AVPicture *) pFrameRGB, context->dec_ctx->width, context->dec_ctx->height, "output.ppm");
}

int main(int argc, char **argv) {
    init();

    jobs = g_hash_table_new(g_str_hash, g_str_equal);

    GtkBuilder *builder;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "builder.ui", NULL);

    result_tree = (GtkWidget *) gtk_builder_get_object(builder, "result_list");
    create_result_list(result_tree);
    g_signal_connect (result_tree, "cursor-changed", G_CALLBACK(row_selected), NULL);

    GObject *button = gtk_builder_get_object(builder, "quit");
    g_signal_connect (button, "clicked", G_CALLBACK(gtk_main_quit), NULL);

    GObject *analyse_button = gtk_builder_get_object(builder, "analyse");
    g_signal_connect (analyse_button, "clicked", G_CALLBACK(analyse), NULL);

    filter_test_entry = GTK_ENTRY (gtk_builder_get_object(builder, "filter_test"));
    g_signal_connect (filter_test_entry, "changed", G_CALLBACK(filter), NULL);

    filter_input_entry = GTK_ENTRY (gtk_builder_get_object(builder, "filter_input"));
    g_signal_connect (filter_input_entry, "changed", G_CALLBACK(filter), NULL);

    start_entry = GTK_ENTRY (gtk_builder_get_object(builder, "start"));
    end_entry = GTK_ENTRY (gtk_builder_get_object(builder, "end"));

    GtkFileChooserButton *input_chooser_button = (GtkFileChooserButton *) gtk_builder_get_object(builder, "input_chooser");
    g_signal_connect (input_chooser_button, "file-set", G_CALLBACK(update_result_list), NULL);
    input_chooser = GTK_FILE_CHOOSER (input_chooser_button);

    GtkFileChooserButton *output_chooser_button = (GtkFileChooserButton *) gtk_builder_get_object(builder, "input_chooser");
    GtkFileChooser *output_chooser = GTK_FILE_CHOOSER (output_chooser_button);

    analyse_input_chooser = GTK_FILE_CHOOSER (gtk_builder_get_object(builder, "analyse_input_chooser"));

    gtk_file_chooser_set_action(input_chooser, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_file_chooser_set_action(output_chooser, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

    gtk_file_chooser_select_filename(input_chooser, "/media/archive/Serien/Stargate/SG1/Staffel 2/");
    gtk_file_chooser_select_filename(analyse_input_chooser, "/media/archive/Serien/Stargate/SG1/Staffel 4/stargate04.fd");
    update_result_list();

    result_preview = GTK_IMAGE (gtk_builder_get_object(builder, "result_preview"));

    GtkButton *save_button = GTK_BUTTON (gtk_builder_get_object(builder, "save"));
    g_signal_connect (save_button, "clicked", G_CALLBACK(export_jobs), NULL);


    GtkScale *extract_scale = (GtkScale *) gtk_builder_get_object(builder, "extract_scale");
    gtk_range_set_adjustment(GTK_RANGE (extract_scale), gtk_adjustment_new(0, 0, 100, 1, 0, 0));
    g_signal_connect (extract_scale, "change-value", G_CALLBACK(extract_skim), NULL);

    extract_preview = GTK_IMAGE (gtk_builder_get_object(builder, "extract_preview"));
    GtkButton *extract_button = GTK_BUTTON (gtk_builder_get_object(builder, "extract_button"));
    g_signal_connect (extract_button, "clicked", G_CALLBACK(save_extract), NULL);


    GtkWindow *extract_window = GTK_WINDOW (gtk_builder_get_object(builder, "extract"));
    GtkWindow *analyse_window = GTK_WINDOW (gtk_builder_get_object(builder, "window"));

    g_signal_connect (analyse_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


//     GtkWidget *menubar;
//     GtkWidget *filemenu;
//     GtkWidget *file;
//     GtkWidget *quit;
//
//     GtkWidget *vbox;
//
//     GObject *root = gtk_builder_get_object ( builder, "root" );
//
//     vbox = gtk_vbox_new ( FALSE, 0 );
//     gtk_container_add ( GTK_CONTAINER ( root ), vbox );
//
//     menubar = gtk_menu_bar_new();
//     filemenu = gtk_menu_new();
//
//     file = gtk_menu_item_new_with_label ( "File" );
//     quit = gtk_menu_item_new_with_label ( "Quit" );
//
//     gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( file ), filemenu );
//     gtk_menu_shell_append ( GTK_MENU_SHELL ( filemenu ), quit );
//     gtk_menu_shell_append ( GTK_MENU_SHELL ( menubar ), file );
//     gtk_box_pack_start ( GTK_BOX ( vbox ), menubar, FALSE, FALSE, 3 );
//
//     gtk_widget_show_all ( GTK_WIDGET ( root ) );
//
//     g_object_unref ( builder );
    gtk_main();
    return 0;
}


