#include "app.h"

GObject *
app_get_ui_element (App * app, const gchar * name)
{
    const gchar *s;
    GSList *list;

    list = app->objects;

    do {
        if(GTK_IS_BUILDABLE(list->data)) {
            s = gtk_buildable_get_name (list->data);
        }

        if (strcmp (s, name) == 0) {
            return list->data;
        }

    } while (list = g_slist_next (list));

    return NULL;
}

void
app_init (App * app)
{
    GError *err = NULL;
    GtkBuilder *builder;

    builder = gtk_builder_new ();

    gtk_builder_add_from_file (builder,
                               UI_DEFINITIONS_FILE, &err);

    if (err != NULL) {
        g_printerr
            ("Error while loading app definitions file: %s\n",
             err->message);
        g_error_free (err);
        gtk_main_quit ();
    }

    gtk_builder_connect_signals (builder, app);

    app->objects = gtk_builder_get_objects (builder);
}
