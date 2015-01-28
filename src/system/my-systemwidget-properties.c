#include "my-systemwidget.h"

static GtkWidget *preferences_dialog = NULL;

static MySystemWidget *system_widget;

static void
my_system_widget_properties_dialog_setup (GtkBuilder * builder,
                                          GtkWindow * window)
{
    GtkWidget *dialog;

    /* UI callbacks */
    dialog =
        GTK_WIDGET (gtk_builder_get_object (builder, "file_management_dialog"));

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
                                         MySystemWidget * widget)
{
    GtkBuilder *builder;

    if (preferences_dialog != NULL) {
        gtk_window_present (GTK_WINDOW (preferences_dialog));
        return;
    }

    builder = gtk_builder_new ();

    gtk_builder_add_from_resource (builder,
                                   "/org/gtk/myapp/my-system-widget-properties.ui",
                                   NULL);

    my_system_widget_properties_dialog_setup (builder, window);

    g_object_unref (builder);
}
