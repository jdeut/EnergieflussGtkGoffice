#include "app.h"

GObject *
app_get_ui_element (App * app, const gchar * name)
{
    const gchar *s;
    GSList *list;

    list = app->objects;

    do {
        if (GTK_IS_BUILDABLE (list->data)) {
            s = gtk_buildable_get_name (list->data);
        }

        if (strcmp (s, name) == 0) {
            return list->data;
        }

    } while (list = g_slist_next (list));

    g_warning
        ("app_get_ui_element: ui element `%s` doesn't exist in ui file\n",
         name);

    return NULL;
}

void
app_style_init (App * app)
{
    GError *err = NULL;
    GtkCssProvider *style;
    GdkScreen *screen;

    GET_UI_ELEMENT (GtkWidget, window1);

    style = gtk_css_provider_new ();

    gtk_css_provider_load_from_path (style, "./style.css", &err);

    if (err != NULL) {
        g_printerr ("Error while loading style file: %s\n", err->message);
        g_clear_error (&err);
        /*exit (1); */
    }
    else {

        screen = gtk_window_get_screen (GTK_WINDOW (window1));
        gtk_style_context_add_provider_for_screen (screen,
                                                   GTK_STYLE_PROVIDER (style),
                                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
}

void
app_init (App * app)
{
    GError *err = NULL;
    GtkBuilder *builder;

    builder = gtk_builder_new ();

    gtk_builder_add_from_file (builder, UI_DEFINITIONS_FILE, &err);

    if (err != NULL) {
        g_printerr
            ("Error while loading app definitions file: %s\n", err->message);
        g_clear_error (&err);
        gtk_main_quit ();
    }

    app->objects = gtk_builder_get_objects (builder);

    gtk_builder_connect_signals (builder, app);

    app_style_init (app);
}
