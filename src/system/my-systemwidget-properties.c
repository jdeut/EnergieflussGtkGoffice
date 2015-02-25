#include "my-systemwidget.h"

enum
{
    MODEL_SPECIFIC,
    MODEL_GENERIC,
    N_MODEL
};

static gchar *system_model_suffix[N_MODEL] = { "specific", "generic" };

static MySystemModel *system_model[N_MODEL];

enum
{
    WIDGET_SYSTEM_LABEL,
    WIDGET_FILECHOOSER_PIC,
    N_WIDGETS
};

static gchar *widget_names[N_WIDGETS] = { "label_", "filechooserbutton_pic_" };

static GtkWidget *widgets[N_MODEL][N_WIDGETS];

static GtkWidget *preferences_dialog = NULL;
static MySystem *my_system;

void
file_chooser_file_set (GtkFileChooserButton * button, gint * i)
{
    gchar *fn;

    fn = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (button));

    g_return_if_fail (fn != NULL);

    if (GTK_WIDGET (button) == widgets[MODEL_SPECIFIC][WIDGET_FILECHOOSER_PIC])
        g_object_set (system_model[MODEL_SPECIFIC], "picture-path", fn, NULL);
    else
        g_object_set (system_model[MODEL_GENERIC], "picture-path", fn, NULL);
}

static void
my_system_widget_properties_dialog_setup (GtkBuilder * builder,
                                          GtkWindow * window)
{
    GtkWidget *dialog;
    gint i, j;

    GtkFileFilter *filter;


    for (i = 0; i < N_MODEL; i++) {

        for (j = 0; j < N_WIDGETS; j++) {

            gchar *str;

            str =
                g_strdup_printf ("%s%s", widget_names[j],
                                 system_model_suffix[i]);

            widgets[i][j] = GTK_WIDGET (gtk_builder_get_object (builder, str));

            if (j == WIDGET_FILECHOOSER_PIC) {

                gchar *path;

                g_object_get (system_model[i], "picture-path", &path, NULL);

                if(path != NULL) {
                    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(widgets[i][j]), path);
                }

                filter = gtk_file_filter_new ();

                gtk_file_filter_set_name (filter, "jpg");
                gtk_file_filter_add_mime_type (filter, "image/*");

                gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widgets[i][j]),
                                             filter);

                filter = gtk_file_filter_new ();

                gtk_file_filter_set_name (filter, "All files");
                gtk_file_filter_add_pattern (filter, "*");

                gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (widgets[i][j]),
                                             filter);
                gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (widgets[i][j]),
                                             filter);

                g_signal_connect (widgets[i][j], "file-set",
                                  G_CALLBACK (file_chooser_file_set), NULL);
            }
            else if (j == WIDGET_SYSTEM_LABEL) {
                g_object_bind_property (system_model[i], "label", widgets[i][j], "text", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE); 
            }

            g_free (str);
        }
    }

    dialog =
        GTK_WIDGET (gtk_builder_get_object
                    (builder, "system_widget_properties_editor_dialog"));

    g_signal_connect (dialog, "response",
                      G_CALLBACK (gtk_widget_destroy), NULL);

    if (window) {
        gtk_window_set_screen (GTK_WINDOW (dialog),
                               gtk_window_get_screen (window));
    }

    preferences_dialog = dialog;

    g_object_add_weak_pointer (G_OBJECT (dialog),
                               (gpointer *) & preferences_dialog);

    gtk_window_set_transient_for (GTK_WINDOW (dialog), window);

    gtk_widget_show (dialog);
}

void
my_system_widget_properties_dialog_show (GtkWindow * window,
                                         MySystemWidget * system_widget)
{
    GtkBuilder *builder;
    MyTimelineModel *timeline;
    GtkWidget *toplevel;
    guint id;

    if (preferences_dialog != NULL) {
        gtk_window_present (GTK_WINDOW (preferences_dialog));
        return;
    }

    g_return_if_fail (MY_IS_WINDOW (window));

    timeline = my_window_get_timeline (MY_WINDOW (window));

    builder = gtk_builder_new ();

    gtk_builder_add_from_resource (builder,
                                   "/org/gtk/myapp/my-system-widget-properties.ui",
                                   NULL);

    g_object_get (system_widget, "specific-model",
                  &system_model[MODEL_SPECIFIC], "id", &id, NULL);
    g_object_get (system_widget, "generic-model", &system_model[MODEL_GENERIC],
                  "id", &id, NULL);

    my_system = my_timeline_get_system_with_id (timeline, id);

    g_return_if_fail (MY_IS_SYSTEM_MODEL (system_model[MODEL_SPECIFIC]));
    g_return_if_fail (MY_IS_SYSTEM_MODEL (system_model[MODEL_GENERIC]));

    my_system_widget_properties_dialog_setup (builder, window);

    g_object_unref (builder);
}
