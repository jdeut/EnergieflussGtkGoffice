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

enum
{
    POPOVER_BINDING_LABEL,
    N_POPOVER_BINDINGS
};

GBinding *popover_binding[N_POPOVER_BINDINGS];

glong popover_handler_file_set;

static GtkWidget *preferences_dialog = NULL;
static MySystem *my_system;

void
my_system_widget_properties_close (MySystemWidget * self, GtkPopover * popover)
{
    GtkWidget * toplevel;
    SystemSettings ss;
    
    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));
    ss = my_window_get_system_settings (MY_WINDOW (toplevel));

    if(G_IS_BINDING(popover_binding[POPOVER_BINDING_LABEL])) {
        g_binding_unbind (popover_binding[POPOVER_BINDING_LABEL]);
    }

    if (g_signal_handler_is_connected
        (ss.filechooserbutton, popover_handler_file_set)) {
        g_signal_handler_disconnect (ss.filechooserbutton,
                                     popover_handler_file_set);
    }
}

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
my_system_widget_properties_dialog_setup (GtkWindow * window)
{
    GtkWidget *dialog;
    SystemSettings ss;
    gint i, j;

    GtkFileFilter *filter;

    ss = my_window_get_system_settings (MY_WINDOW (window));

    widgets[MODEL_SPECIFIC][WIDGET_SYSTEM_LABEL] = ss.entry;
    widgets[MODEL_SPECIFIC][WIDGET_FILECHOOSER_PIC] = ss.filechooserbutton;

    for (i = 0; i < N_MODEL; i++) {

        for (j = 0; j < N_WIDGETS; j++) {

            if (!GTK_IS_WIDGET (widgets[i][j]))
                continue;

            gchar *str;

            str =
                g_strdup_printf ("%s%s", widget_names[j],
                                 system_model_suffix[i]);

            if (j == WIDGET_FILECHOOSER_PIC) {

                gchar *path;

                g_object_get (system_model[i], "picture-path", &path, NULL);

                if (path != NULL) {
                    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER
                                                   (widgets[i][j]), path);
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

                popover_handler_file_set =
                    g_signal_connect (widgets[i][j], "file-set",
                                      G_CALLBACK (file_chooser_file_set), NULL);
            }
            else if (j == WIDGET_SYSTEM_LABEL) {
                gtk_entry_set_text(GTK_ENTRY(widgets[i][j]), "");

                popover_binding[POPOVER_BINDING_LABEL] =
                    g_object_bind_property (system_model[i], "label",
                                            widgets[i][j], "text",
                                            G_BINDING_BIDIRECTIONAL |
                                            G_BINDING_SYNC_CREATE);
            }

            g_free (str);
        }
    }
}

void
my_system_widget_properties_dialog_show (GtkWindow * window,
                                         MySystemWidget * system_widget)
{
    MyTimelineModel *timeline;
    guint id;

    g_return_if_fail (MY_IS_WINDOW (window));

    timeline = my_window_get_timeline (MY_WINDOW (window));

    g_object_get (system_widget, "specific-model",
                  &system_model[MODEL_SPECIFIC], "id", &id, NULL);
    g_object_get (system_widget, "generic-model", &system_model[MODEL_GENERIC],
                  "id", &id, NULL);

    my_system = my_timeline_get_system_with_id (timeline, id);

    g_return_if_fail (MY_IS_SYSTEM_MODEL (system_model[MODEL_SPECIFIC]));
    g_return_if_fail (MY_IS_SYSTEM_MODEL (system_model[MODEL_GENERIC]));

    my_system_widget_properties_dialog_setup (window);
}
