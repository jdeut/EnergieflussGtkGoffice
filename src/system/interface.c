#include "interface.h"

GObject *
interface_get_ui_element (Interface * iface, const gchar * name)
{
    const gchar *s;
    GSList *list;

    list = iface->objects;

    do {
        if (GTK_IS_BUILDABLE (list->data)) {
            s = gtk_buildable_get_name (list->data);
        }

        if (strcmp (s, name) == 0) {
            return list->data;
        }

    } while (list = g_slist_next (list));

    g_warning
        ("interface_get_ui_element: ui element `%s` doesn't exist in ui file\n",
         name);

    return NULL;
}

void
interface_style_init (Interface * iface)
{
    GError *err = NULL;
    GFile *css_file;
    GtkCssProvider *style;
    GdkScreen *screen;

    GET_UI_ELEMENT (GtkWidget, applicationwindow);

    style = gtk_css_provider_new ();

    css_file = g_file_new_for_uri ("resource:///org/gtk/myapp/style.css");
    gtk_css_provider_load_from_file (style, css_file, &err);

    if (err != NULL) {
        g_printerr ("Error while loading style file: %s\n", err->message);
        g_clear_error (&err);
        /*exit (1); */
    }
    else {

        screen = gtk_window_get_screen (GTK_WINDOW (applicationwindow));
        gtk_style_context_add_provider_for_screen (screen,
                                                   GTK_STYLE_PROVIDER (style),
                                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
}

void
interface_init (Interface * iface)
{
    GError *err = NULL;
    GtkBuilder *builder;

    builder = gtk_builder_new ();

    gtk_builder_add_from_resource (builder, "/org/gtk/myapp/window.ui", &err);

    if (err != NULL) {
        g_printerr
            ("Error while loading iface definitions file: %s\n", err->message);
        g_clear_error (&err);
        gtk_main_quit ();
    }

    iface->objects = gtk_builder_get_objects (builder);

    gtk_builder_connect_signals (builder, iface);

    interface_style_init (iface);
}

void
interface_populate_canvas (Interface * iface)
{
    GocGroup *group_systems, *group_arrows;
    GocItem *system1, *system2, *arrow;

    group_systems = iface->canvas->group_systems;
    group_arrows = iface->canvas->group_arrows;

    system1 =
        goc_item_new (group_systems, MY_TYPE_SYSTEM, "x", 100.0, "y", 200.0,
                      NULL);

    arrow =
        goc_item_new (group_arrows, MY_TYPE_FLOW_ARROW, "energy-quantity", 10.0,
                      "linked-system", system1, "secondary-system", NULL,
                      "anchor", MY_ANCHOR_EAST, NULL);

    system2 =
        goc_item_new (group_systems, MY_TYPE_SYSTEM, "x", 300.0, "y", 400.0,
                      NULL);
    arrow =
        goc_item_new (group_arrows, MY_TYPE_FLOW_ARROW, "energy-quantity", 10.0,
                      "linked-system", system1, "secondary-system", system2,
                      "anchor", MY_ANCHOR_EAST, NULL);
}

void
interface_create (MyApplication * app)
{
    Interface *iface;

    iface = (Interface *) g_new (Interface, 1);

    interface_init (iface);

    GET_UI_ELEMENT (GtkWidget, applicationwindow);
    GET_UI_ELEMENT (GtkBox, scrolledwindow1);

    gtk_window_set_role (GTK_WINDOW (applicationwindow), "MyGtkTraining");

    iface->canvas = g_object_new (MY_TYPE_CANVAS, "expand", TRUE, NULL);

    gtk_container_add (GTK_CONTAINER (scrolledwindow1),
                       GTK_WIDGET (iface->canvas));

    interface_populate_canvas (iface);

    my_application_set_window (app, applicationwindow);
    my_application_set_interface (app, iface);
}
