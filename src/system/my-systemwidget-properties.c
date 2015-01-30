#include "my-systemwidget.h"

static GtkWidget *preferences_dialog = NULL;

static MySystemModel *system_model;
static MySystem *my_system;

void
file_chooser_file_set (GtkFileChooserButton * button, gpointer data)
{
    gchar *fn;

    fn = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (button));

    g_return_if_fail(fn != NULL);

    g_object_set (system_model, "picture-path", fn, NULL);
}

static void
my_system_widget_properties_dialog_setup (GtkBuilder * builder,
                                          GtkWindow * window)
{
    GtkWidget *file_chooser;
    GtkWidget *dialog;
    GtkWidget *system_label;

    GtkFileFilter *filter;

    system_label =
        GTK_WIDGET (gtk_builder_get_object
                    (builder, "system_label"));

    dialog =
        GTK_WIDGET (gtk_builder_get_object
                    (builder, "system_widget_properties_editor_dialog"));

    file_chooser =
        GTK_WIDGET (gtk_builder_get_object (builder, "filechooserbutton_pic"));

    g_object_bind_property (my_system, "label", system_label, "text",
                            G_BINDING_BIDIRECTIONAL |
                            G_BINDING_SYNC_CREATE);

    g_signal_connect (dialog, "response",
                      G_CALLBACK (gtk_widget_destroy), NULL);

    if (window) {
        gtk_window_set_screen (GTK_WINDOW (dialog),
                               gtk_window_get_screen (window));
    }

    preferences_dialog = dialog;

    g_object_add_weak_pointer (G_OBJECT (dialog),
                               (gpointer *) & preferences_dialog);

    filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter, "jpg");
    gtk_file_filter_add_mime_type (filter, "image/*");

    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser), filter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (file_chooser), filter);

    filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter, "All files");
    gtk_file_filter_add_pattern (filter, "*");

    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser), filter);

    g_signal_connect (file_chooser, "file-set",
                      G_CALLBACK (file_chooser_file_set), NULL);

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

    timeline = my_window_get_timeline (MY_WINDOW(window));

    builder = gtk_builder_new ();

    gtk_builder_add_from_resource (builder,
                                   "/org/gtk/myapp/my-system-widget-properties.ui",
                                   NULL);

    g_object_get (system_widget, "model", &system_model, "id", &id, NULL);

    my_system = my_timeline_get_system_with_id (timeline, id);

    g_return_if_fail (MY_IS_SYSTEM_MODEL (system_model));

    my_system_widget_properties_dialog_setup (builder, window);

    g_object_unref (builder);
}
