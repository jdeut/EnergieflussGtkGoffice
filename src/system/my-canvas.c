#include "my-canvas.h"

#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

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
    guint add_arrow_mode;
    guint add_system_mode;
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

static gboolean
my_canvas_drag_drag_point (MyCanvas * self, gdouble x, gdouble y)
{
    goc_item_set (self->_priv->active_item, "x", x, "y", y, NULL);
}


static gboolean
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

    /* if in ADD SYSTEM MODE */
    if (self->_priv->add_system_mode && !GOC_IS_ITEM (self->_priv->active_item)) {

        goc_item_new (MY_CANVAS (canvas)->group_systems, MY_TYPE_SYSTEM, "x",
                      x_cv, "y", y_cv, NULL);

        self->_priv->add_system_mode = FALSE;
    }
    /* if in ADD ARROW MODE */
    else if (self->_priv->add_arrow_mode
             && MY_IS_SYSTEM (self->_priv->active_item)) {
        MyFlowArrow *arrow;
        MyDragPoint *point;

        arrow =
            (MyFlowArrow *) goc_item_new (MY_CANVAS (self)->group_arrows,
                                          MY_TYPE_FLOW_ARROW, "linked-system",
                                          self->_priv->active_item, "anchor",
                                          MY_ANCHOR_EAST, "x1", x_cv, "y1",
                                          y_cv, "x0", x_cv, "y0", y_cv, NULL);

        my_flow_arrow_show_drag_points (arrow);

        point = my_flow_arrow_get_drag_point (arrow);

        goc_item_set (GOC_ITEM (point), "x", x_cv, "y", y_cv, NULL);

        self->_priv->active_item = GOC_ITEM (point);
    }
    else {
        self->_priv->add_arrow_mode = FALSE;
    }

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

static gboolean
my_canvas_button_press_3_cb (GocCanvas * canvas, GdkEventButton * event,
                             gpointer data)
{
    GdkEventButton *event_button;

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
    self->_priv->add_arrow_mode = FALSE;
    self->_priv->add_system_mode = FALSE;

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

/* begin public functions */

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

void
my_canvas_add_system (MyCanvas * self)
{
    g_return_if_fail (MY_IS_CANVAS (self));

    self->_priv->add_system_mode = TRUE;
}

gboolean
my_canvas_generate_json_data_stream (MyCanvas * self, gchar ** str, gsize * len)
{
    JsonGenerator *gen = json_generator_new ();
    JsonArray *array_arrows, *array_systems, *array_root;
    JsonNode *node, *root, *arrows, *systems;
    GocGroup *group_arrows, *group_systems;
    GList *l;

    group_arrows = self->group_arrows;
    group_systems = self->group_systems;

    root = json_node_new (JSON_NODE_ARRAY);
    arrows = json_node_new (JSON_NODE_ARRAY);
    systems = json_node_new (JSON_NODE_ARRAY);

    array_root = json_array_new ();
    array_arrows = json_array_new ();
    array_systems = json_array_new ();

    json_node_set_array (root, array_root);
    json_node_set_array (arrows, array_arrows);
    json_node_set_array (systems, array_systems);

    json_array_add_element (array_root, arrows);
    json_array_add_element (array_root, systems);

    for (l = group_arrows->children; l != NULL; l = l->next) {
        if (MY_IS_FLOW_ARROW (l->data)) {
            node = json_gobject_serialize (G_OBJECT (l->data));
            json_array_add_element (array_arrows, node);
        }
    }
    for (l = group_systems->children; l != NULL; l = l->next) {
        if (MY_IS_SYSTEM (l->data)) {
            node = json_gobject_serialize (G_OBJECT (l->data));
            json_array_add_element (array_systems, node);
        }
    }

    json_generator_set_root (gen, root);
    json_generator_set_pretty (gen, TRUE);

    *str = json_generator_to_data (gen, len);

    json_node_free (root);

    json_array_unref (array_systems);
    json_array_unref (array_arrows);
    json_array_unref (array_root);

    return TRUE;
}

void
my_canvas_add_flow_arrow (MyCanvas * self)
{
    g_return_if_fail (MY_IS_CANVAS (self));

    self->_priv->add_arrow_mode = TRUE;
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

gboolean
my_canvas_button_release_cb (GocCanvas * canvas, GdkEvent * event,
                             gpointer data)
{
    MyCanvas *self = MY_CANVAS (canvas);
    MySystem *linked_system;
    MyFlowArrow *arrow;
    GocItem *item;

    if (self->_priv->add_arrow_mode) {
        self->_priv->add_arrow_mode = FALSE;
    }

    if (MY_IS_DRAG_POINT (self->_priv->active_item)) {

        gdouble d =
            goc_item_distance (GOC_ITEM (self->group_systems), event->button.x,
                               event->button.y, &item);

        g_object_get (self->_priv->active_item, "linked-item", &arrow, NULL);

        if (MY_IS_FLOW_ARROW (arrow)) {

            g_object_get (arrow, "linked-system", &linked_system, NULL);

            /* only do it if drag point is over a system but not over the system the corresponding arrow is linked with */

            if (d == 0. && MY_IS_SYSTEM (item)
                && MY_SYSTEM (linked_system) != MY_SYSTEM (item)) {

                g_object_set (arrow, "secondary-system", item, NULL);

            }
            else {
                g_object_set (arrow, "secondary-system", NULL, NULL);
            }
        }

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
            my_canvas_drag_drag_point (self, x_item_new, y_item_new);
        }
        else if (GOC_IS_CIRCLE (active_item)) {
            goc_item_set (active_item, "x", x_item_new, "y", y_item_new, NULL);
        }

        goc_item_invalidate (active_item);
        gtk_widget_queue_draw (GTK_WIDGET (canvas));
    }
    return TRUE;
}
