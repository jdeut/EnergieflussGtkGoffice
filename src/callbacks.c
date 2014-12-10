#include "app.h"

gdouble offsetx, offsety;

gboolean
propagate_button_release_event_to_canvas_cb (GtkWidget * widget,
                                             GdkEvent * event, App * app)
{
    button_release_cb (GTK_WIDGET (app->canvas), event, app);

    return FALSE;
}

gboolean
propagate_motion_notify_event_to_canvas_cb (GtkWidget * widget,
                                            GdkEventMotion * event, App * app)
{
    GdkEventMotion e;
    gint x, y;

    gtk_widget_translate_coordinates (widget,
                                      GTK_WIDGET (app->canvas),
                                      event->x, event->y, &x, &y);

    event->x = x;
    event->y = y;

    motion_notify_cb (GTK_WIDGET (app->canvas), event, app);

    return FALSE;
}

gboolean
propagate_button_press_event_to_canvas_cb (GtkWidget * widget,
                                           GdkEventButton * event, App * app)
{
    GdkEventButton e;
    gint x, y;

    e = *event;

    gtk_widget_translate_coordinates (widget,
                                      GTK_WIDGET (app->canvas),
                                      event->x, event->y, &x, &y);

    e.x = x;
    e.y = y;
    e.window = gtk_layout_get_bin_window (&app->canvas->base);

    button_press_cb (GTK_WIDGET (app->canvas), &e, app);

    return FALSE;
}

gboolean
button_release_cb (GtkWidget * widget, GdkEvent * event, App * app)
{
    app->active_item = NULL;
    return TRUE;
}

gboolean
motion_notify_cb (GtkWidget * widget, GdkEventMotion * event, App * app)
{
    cairo_matrix_t matrix;
    gdouble tx, ty;
    gdouble x, y;

    x = event->x;
    y = event->y;

    if (app->active_item != NULL) {
        tx = x - offsetx;
        ty = y - offsety;

        if (GOC_IS_LINE (app->active_item)) {
            gdouble x0, y0, x1, y1;

            g_object_get (app->active_item, "x0", &x0, "y0", &y0, NULL);
            g_object_get (app->active_item, "x1", &x1, "y1", &y1, NULL);

            goc_item_set (app->active_item, "x0", x0 + tx, "y0", y0 + ty, NULL);
            goc_item_set (app->active_item, "x1", x1 + tx, "y1", y1 + ty, NULL);
        }
        else if (GOC_IS_ARC (app->active_item)) {
            gdouble xc, yc;

            g_object_get (app->active_item, "xc", &xc, "yc", &yc, NULL);

            goc_item_set (app->active_item, "xc", xc + tx, "yc", yc + ty, NULL);
        }
        else if (GOC_IS_WIDGET (app->active_item)) {
            gdouble x, y;

            g_object_get (app->active_item, "x", &x, "y", &y, NULL);

            goc_item_set (app->active_item, "x", x + tx, "y", y + ty, NULL);
        }

        goc_item_invalidate (app->active_item);
        gtk_widget_queue_draw (GTK_WIDGET (app->canvas));

        offsetx = x;
        offsety = y;
    }
    return TRUE;
}

void
button_press_1_cb (GtkWidget * widget, GdkEventButton * event, App * app)
{
    gdouble x, y;

    x = event->x;
    y = event->y;

    if (event->window != gtk_layout_get_bin_window (&app->canvas->base))
        return;

    offsetx = (app->canvas->direction == GOC_DIRECTION_RTL) ?
        app->canvas->scroll_x1 + (app->canvas->width -
                                  x) /
        app->canvas->pixels_per_unit : app->canvas->scroll_x1 +
        x / app->canvas->pixels_per_unit;

    offsety = app->canvas->scroll_y1 + y / app->canvas->pixels_per_unit;

    app->active_item = goc_canvas_get_item_at (app->canvas, offsetx, offsety);

    if (app->active_item) {
        g_print ("hit\n");
    }
}

gboolean
button_press_3_cb (GtkWidget * widget, GdkEventButton * event, App * app)
{
    GdkEventButton *event_button;

    GET_UI_ELEMENT (GtkMenu, CanvasMenu);

    if (event->type == GDK_BUTTON_PRESS) {
        event_button = (GdkEventButton *) event;
        if (event_button->button == GDK_BUTTON_SECONDARY) {
            gtk_menu_popup (CanvasMenu, NULL, NULL, NULL, NULL,
                            event_button->button, event_button->time);
            return TRUE;
        }
    }
    return FALSE;
}

gboolean
button_press_cb (GtkWidget * widget, GdkEventButton * event, App * app)
{
    if (event->button == 1) {
        button_press_1_cb (widget, event, app);
        return TRUE;
    }
    else if (event->button == 3) {
        return button_press_3_cb (widget, event, app);
    }
}

void
button_add_clicked_cb (GtkWidget * widget, App * app)
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

    top_level_group = goc_canvas_get_root (app->canvas);

    button = gtk_button_new_with_label ("System");

    g_print ("x: %f y: %f\n", (gdouble) src_x, (gdouble) src_y);

    offsetx = src_x + 50;
    offsety = src_y + 25;

    item =
        goc_item_new (top_level_group, GOC_TYPE_WIDGET, "widget", button,
                      "x", (gdouble) src_x, "y", (gdouble) src_y, "width",
                      100.0, "height", 50.0, NULL);

    app->active_item = item;

    g_signal_connect (button, "button-press-event",
                      G_CALLBACK
                      (propagate_button_press_event_to_canvas_cb), app);
    g_signal_connect (button, "button-release-event",
                      G_CALLBACK
                      (propagate_button_release_event_to_canvas_cb), app);
    g_signal_connect (button, "motion-notify-event",
                      G_CALLBACK
                      (propagate_motion_notify_event_to_canvas_cb), app);
}
