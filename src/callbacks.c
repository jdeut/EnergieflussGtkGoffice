#include "app.h"

G_MODULE_EXPORT void show_drag_points_cb (GtkWidget * widget, App * app) {

    my_canvas_show_drag_points_of_all_arrows(app->canvas);
}

G_MODULE_EXPORT void button_add_arrow_clicked_cb (GtkWidget * widget, App * app) {

    guint contextid;
    gchar *msg = {"Message"};
    
    GET_UI_ELEMENT(GtkStatusbar, statusbar1);

    contextid = gtk_statusbar_get_context_id(statusbar1, msg);

    gtk_statusbar_push(statusbar1, contextid, msg);
}

G_MODULE_EXPORT void
button_add_system_clicked_cb (GtkWidget * widget, App * app)
{

    GocGroup *top_level_group;
    GocItem *item;
    gint src_x, src_y;
    GdkDeviceManager *manager;
    GdkDevice *device;
    GdkWindow *window;
    GtkWidget *button;

    GET_UI_ELEMENT (GtkWidget, window1);

    window = gtk_widget_get_window (GTK_WIDGET(app->canvas));
    manager = gdk_display_get_device_manager (gdk_display_get_default ());
    device = gdk_device_manager_get_client_pointer (manager);

    gdk_window_get_device_position (window, device, &src_x, &src_y, NULL);

    top_level_group = goc_canvas_get_root (GOC_CANVAS(app->canvas));

    button = gtk_button_new_with_label ("System");

    g_print ("x: %f y: %f\n", (gdouble) src_x, (gdouble) src_y);

}
