#include "my-canvas.h"

/* 'private'/'protected' functions */
static void my_canvas_class_init (MyCanvasClass * klass);
static void my_canvas_init (MyCanvas * self);
static void my_canvas_finalize (GObject *);
static void my_canvas_dispose (GObject *);

struct _MyCanvasPrivate
{
    /* private members go here */
    GocItem *active_item;
    gdouble offsetx, offsety;
};

#define MY_CANVAS_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                       MY_TYPE_CANVAS, \
                                       MyCanvasPrivate))

G_DEFINE_TYPE (MyCanvas, my_canvas, GOC_TYPE_CANVAS);


GQuark
my_canvas_error_quark (void)
{
    return g_quark_from_static_string ("my-canvas-error-quark");
}

gboolean
my_canvas_button_release_cb (GocCanvas * canvas, GdkEvent * event,
                             gpointer data)
{
    MyCanvas *self = MY_CANVAS (canvas);

    if (MY_IS_DRAG_POINT (self->_priv->active_item)) {
        my_drag_point_end_dragging (MY_DRAG_POINT (self->_priv->active_item));
    }

    self->_priv->active_item = NULL;

    return TRUE;
}

gboolean
my_canvas_motion_notify_cb (GocCanvas * canvas, GdkEventMotion * event,
                            gpointer data)
{
    MyCanvas *self = MY_CANVAS (canvas);
    GocItem *active_item = self->_priv->active_item;

    cairo_matrix_t matrix;
    gdouble x_cv, y_cv;

    /* coordinates on canvas */
    x_cv = event->x;
    y_cv = event->y;

    if (active_item != NULL) {

        gdouble x_item_old, y_item_old;
        gdouble x_item_new, y_item_new;

        if (GOC_IS_WIDGET (active_item)) {

            gint x, y;

            g_return_val_if_fail (GTK_IS_WIDGET (data), FALSE);

            gtk_widget_translate_coordinates (GTK_WIDGET (data),
                                              GTK_WIDGET (canvas),
                                              event->x, event->y, &x, &y);

            /* coordinates on canvas */
            x_cv = x;
            y_cv = y;
        }

        g_object_get (active_item, "x", &x_item_old, "y", &y_item_old, NULL);

        x_item_new = x_cv - self->_priv->offsetx;
        y_item_new = y_cv - self->_priv->offsety;

        if (GOC_IS_WIDGET (active_item)) {
            goc_item_set (active_item, "x", x_item_new, "y", y_item_new, NULL);
        }
        else if (MY_IS_DRAG_POINT (active_item)) {
            GocItem *item;

            gdouble d =
                goc_item_distance (GOC_ITEM (self->group_systems), x_item_new,
                                   y_item_new, &item);

            if (d == 0. && MY_IS_SYSTEM (item)) {
                g_print ("kok\n");
            }

            goc_item_set (active_item, "x", x_item_new, "y", y_item_new, NULL);
        }
        else if (GOC_IS_CIRCLE (active_item)) {
            goc_item_set (active_item, "x", x_item_new, "y", y_item_new, NULL);
        }

        goc_item_invalidate (active_item);
        gtk_widget_queue_draw (GTK_WIDGET (canvas));
    }
    return TRUE;
}

gboolean
my_canvas_button_press_1_cb (GocCanvas * canvas, GdkEventButton * event,
                             gpointer data)
{
    MyCanvas *self = MY_CANVAS (canvas);

    gdouble offsetx, offsety;
    gdouble x_cv, y_cv, dx, dy;

    /* coordinates on canvas */
    x_cv = event->x;
    y_cv = event->y;

    if (event->window != gtk_layout_get_bin_window (&canvas->base)) {
        gint x, y;

        gtk_widget_translate_coordinates (GTK_WIDGET (data),
                                          GTK_WIDGET (canvas),
                                          event->x, event->y, &x, &y);

        /* coordinates on canvas */
        x_cv = x;
        y_cv = y;
    }

    dx = event->x;
    dy = event->y;

    self->_priv->active_item = goc_canvas_get_item_at (canvas, x_cv, y_cv);

    if (GOC_IS_CIRCLE (self->_priv->active_item)
        || MY_IS_DRAG_POINT (self->_priv->active_item)) {

        gdouble x, y;

        g_object_get (self->_priv->active_item, "x", &x, "y", &y, NULL);

        dx = event->x - x;
        dy = event->y - y;
    }

    self->_priv->offsetx = (canvas->direction == GOC_DIRECTION_RTL) ?
        canvas->scroll_x1 + (canvas->width -
                             dx) /
        canvas->pixels_per_unit : canvas->scroll_x1 +
        dx / canvas->pixels_per_unit;

    self->_priv->offsety = canvas->scroll_y1 + dy / canvas->pixels_per_unit;

    if (MY_IS_DRAG_POINT (self->_priv->active_item)) {
        my_drag_point_begin_dragging (MY_DRAG_POINT (self->_priv->active_item));
    }

    return FALSE;
}

gboolean
my_canvas_button_press_3_cb (GocCanvas * canvas, GdkEventButton * event,
                             gpointer data)
{
    GdkEventButton *event_button;

    /*GET_UI_ELEMENT (GtkMenu, CanvasMenu); */

    /*if (event->type == GDK_BUTTON_PRESS) { */
    /*event_button = (GdkEventButton *) event; */
    /*if (event_button->button == GDK_BUTTON_SECONDARY) { */
    /*gtk_menu_popup (CanvasMenu, NULL, NULL, NULL, NULL, */
    /*event_button->button, event_button->time); */
    /*return TRUE; */
    /*} */
    /*} */
    return FALSE;
}

gboolean
my_canvas_button_press_cb (GocCanvas * canvas, GdkEventButton * event,
                           gpointer data)
{
    if (event->button == 1) {
        my_canvas_button_press_1_cb (canvas, event, data);
        return TRUE;
    }
    else if (event->button == 3) {
        return my_canvas_button_press_3_cb (canvas, event, data);
    }

    return FALSE;
}

static void
my_canvas_class_init (MyCanvasClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_canvas_finalize;
    gobject_class->dispose = my_canvas_dispose;

    g_type_class_add_private (gobject_class, sizeof (MyCanvasPrivate));
}

static void
my_canvas_init (MyCanvas * self)
{
    GocGroup *root;

    root = goc_canvas_get_root (GOC_CANVAS (self));

    self->_priv = MY_CANVAS_GET_PRIVATE (self);
    self->_priv->active_item = NULL;

    self->group_arrows = goc_group_new (root);
    self->group_systems = goc_group_new (root);

    g_signal_connect (G_OBJECT (self), "button-press-event",
                      G_CALLBACK (my_canvas_button_press_cb), NULL);
    g_signal_connect (self, "button-release-event",
                      G_CALLBACK (my_canvas_button_release_cb), NULL);
    g_signal_connect (self, "motion-notify-event",
                      G_CALLBACK (my_canvas_motion_notify_cb), NULL);
}

static void
my_canvas_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_canvas_parent_class)->dispose (object);
}

static void
my_canvas_finalize (GObject * object)
{
    G_OBJECT_CLASS (my_canvas_parent_class)->finalize (object);
}

MyCanvas *
my_canvas_new (void)
{
    MyCanvas *self;

    self = g_object_new (MY_TYPE_CANVAS, NULL);

    return self;
}

void
my_canvas_show_drag_points_of_all_arrows (MyCanvas * self)
{
    GList *l;
    GocGroup *group;

    group = self->group_arrows;

    for (l = group->children; l != NULL; l = l->next) {
        if (MY_IS_FLOW_ARROW (l->data)) {
            g_print ("it is an arrow :)\n");
            my_flow_arrow_show_drag_points (MY_FLOW_ARROW (l->data));
        }
    }
}
