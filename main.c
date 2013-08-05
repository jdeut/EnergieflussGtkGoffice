#include "app.h"

int
main (int argc, char *argv[])
{
    App *app;

    app = (App *) g_new (App, 1);
    gtk_init (&argc, &argv);

    app_init (app);

    GET_UI_ELEMENT (GtkWidget, window1);

    gtk_window_set_wmclass (GTK_WINDOW (window1),
                            "MyGtkTraining", "MyGtkTraining");

    gtk_widget_show_all (window1);

    gtk_main ();

    return 0;
}
