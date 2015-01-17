#include "system/interface.h"

G_MODULE_EXPORT void
show_drag_points_cb (GtkWidget * widget, Interface * iface)
{
    my_canvas_show_drag_points_of_all_arrows (iface->canvas);
}

G_MODULE_EXPORT void
button_add_arrow_clicked_cb (GtkWidget * widget, Interface * iface)
{
    guint contextid;
    gchar *msg = { "Click the system to which you want to add an energy flow" };

    GET_UI_ELEMENT (GtkStatusbar, statusbar1);

    contextid = gtk_statusbar_get_context_id (statusbar1, msg);

    gtk_statusbar_push (statusbar1, contextid, msg);

    my_canvas_add_flow_arrow (iface->canvas);
}

G_MODULE_EXPORT void
button_add_system_clicked_cb (GtkWidget * widget, Interface * iface)
{
    my_canvas_add_system (iface->canvas);
}

G_MODULE_EXPORT void button_quit_clicked_cb (GtkWidget *widget, Interface * iface) {

}
