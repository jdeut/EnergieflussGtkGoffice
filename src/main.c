#include "app.h"

void
populate_canvas (App * app)
{
    GocGroup *group_systems;
    GocItem *item1, *item2;

    GtkWidget *button;

    group_systems = app->canvas->group_systems;

    item1 =
        goc_item_new (group_systems, MY_TYPE_SYSTEM, "x", 100.0, "y", 200.0,
                      NULL);

    my_system_add_energy_transfer_to_environment (MY_SYSTEM (item1), NULL,
                                                  ANCHOR_WEST, 10.0);
    my_system_add_energy_transfer_to_environment (MY_SYSTEM (item1), NULL,
                                                  ANCHOR_SOUTH, 30.0);
    my_system_add_energy_transfer_to_environment (MY_SYSTEM (item1), NULL,
                                                  ANCHOR_EAST, 5.0);
    my_system_add_energy_transfer_to_environment (MY_SYSTEM (item1), NULL,
                                                  ANCHOR_NORTH, 2.0);

    item1 =
        goc_item_new (group_systems, MY_TYPE_SYSTEM, "x", 100.0, "y", 100.0,
                      NULL);

    item2 =
        goc_item_new (group_systems, MY_TYPE_SYSTEM, "x", 500.0, "y", 100.0,
                      NULL);

    my_system_add_energy_transfer_to_system (MY_SYSTEM (item2),
                                             "<span size=\"xx-large\">W<sub>el</sub></span>",
                                             ANCHOR_EAST, 30,
                                             MY_SYSTEM (item1));

    item1 =
        goc_item_new (group_systems, GOC_TYPE_CIRCLE, "x", 100.0, "y", 200.0, "radius", 20.0,
                      NULL);
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

    /*g_signal_connect (app->canvas, "button-press-event",*/
                      /*G_CALLBACK (button_press_cb), app);*/
    /*g_signal_connect (app->canvas, "button-release-event",*/
                      /*G_CALLBACK (button_release_cb), app);*/
    /*g_signal_connect (app->canvas, "motion-notify-event",*/
                      /*G_CALLBACK (motion_notify_cb), app);*/

    gtk_main ();

    libgoffice_shutdown ();

    return 0;
}
