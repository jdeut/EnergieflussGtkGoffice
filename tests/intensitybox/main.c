#include <gtk/gtk.h>

#include "../../src/system/my-intensitybox.h"

int
main (int argc, char *argv[])
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *grid;
    MyIntensityBox *ib;

    gtk_init (&argc, &argv);

    ib = my_intensity_box_new ();

    gtk_widget_set_vexpand (GTK_WIDGET (ib), TRUE);
    gtk_widget_set_hexpand (GTK_WIDGET (ib), TRUE);

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "builder.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    window = (GtkWidget *) gtk_builder_get_object (builder, "window");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_set_role (GTK_WINDOW (window), "MyGtkTest");
    gtk_window_set_wmclass (GTK_WINDOW(window), "MyGtkTest", "MyGtkTest");

    grid = (GtkWidget *) gtk_builder_get_object (builder, "grid1");

    g_return_if_fail (GTK_IS_GRID (grid));

    gtk_grid_attach (GTK_GRID (grid), GTK_WIDGET (ib), 1, 1, 1, 1);

    gtk_widget_show_all (window);

    gtk_main ();

    return 0;
}
