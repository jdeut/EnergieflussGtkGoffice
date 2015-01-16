#include "app.h"

void
populate_canvas (App * app)
{
    GocGroup *group_systems, *group_arrows;
    GocItem *system1, *system2, *arrow;

    GtkWidget *button;

    group_systems = app->canvas->group_systems;
    group_arrows = app->canvas->group_arrows;

    system1 =
        goc_item_new (group_systems, MY_TYPE_SYSTEM, "x", 100.0, "y", 200.0,
                      NULL);

    arrow =
        goc_item_new (group_arrows, MY_TYPE_FLOW_ARROW, "energy-quantity", 10.0,
                      "linked-system", system1, "secondary-system", NULL, "anchor", MY_ANCHOR_WEST, NULL);

    system2 =
        goc_item_new (group_systems, MY_TYPE_SYSTEM, "x", 300.0, "y", 400.0,
                      NULL);
    arrow =
        goc_item_new (group_arrows, MY_TYPE_FLOW_ARROW, "energy-quantity", 10.0,
                      "linked-system", system1, "secondary-system", system2, "anchor", MY_ANCHOR_EAST, NULL);
}

int
main (int argc, char *argv[])
{
    App *app;

    app = (App *) g_new (App, 1);

    libgoffice_init ();

    gtk_init (&argc, &argv);

    app_init (app);

    GET_UI_ELEMENT (GtkWidget, window1);
    GET_UI_ELEMENT (GtkBox, scrolledwindow1);

    gtk_window_set_role (GTK_WINDOW (window1), "MyGtkTraining");

    app->canvas = g_object_new (MY_TYPE_CANVAS, "expand", TRUE, NULL);

    gtk_container_add (GTK_CONTAINER (scrolledwindow1),
                       GTK_WIDGET (app->canvas));

    populate_canvas (app);

    gtk_widget_show_all (window1);

    gtk_main ();

    libgoffice_shutdown ();

    return 0;
}
