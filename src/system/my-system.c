#include "my-system.h"

#include <json-glib/json-glib.h>

/******************************************************************************
 * GocOffscreenBox: code mostly copied from gtk+/tests/gtkoffscreenbox.[c,h]
 ******************************************************************************/

#define GOC_TYPE_OFFSCREEN_BOX              (goc_offscreen_box_get_type ())
#define GOC_OFFSCREEN_BOX(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOC_TYPE_OFFSCREEN_BOX, GocOffscreenBox))
#define GOC_OFFSCREEN_BOX_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GOC_TYPE_OFFSCREEN_BOX, GocOffscreenBoxClass))
#define GOC_IS_OFFSCREEN_BOX(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOC_TYPE_OFFSCREEN_BOX))
#define GOC_IS_OFFSCREEN_BOX_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GOC_TYPE_OFFSCREEN_BOX))
#define GOC_OFFSCREEN_BOX_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GOC_TYPE_OFFSCREEN_BOX, GocOffscreenBoxClass))

typedef struct _GocOffscreenBox GocOffscreenBox;
typedef GtkBinClass GocOffscreenBoxClass;

static GObjectClass *goc_offscreen_box_parent_class;

struct _GocOffscreenBox
{
    GtkBin base;
    GtkWidget *child;
    GdkWindow *offscreen_window;
    gdouble angle, scale;
};

struct _GocOffscreenBoxClass
{
    GtkBinClass parent_class;
};

/******************************************************************************
 * GocOffscreenBox: End
 ******************************************************************************/

enum
{
    PROP_0,
    PROP_ID,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* 'private'/'protected' functions */
static void my_system_class_init (MySystemClass * klass);
static void my_system_init (MySystem * self);
static void my_system_finalize (GObject *);
static void my_system_dispose (GObject *);

enum
{
    DRAG_POINT_NE,
    DRAG_POINT_NW,
    DRAG_POINT_SE,
    DRAG_POINT_SW,
    N_DRAG_POINTS
};

enum
{
    SYSTEM_CHANGED_X,
    SYSTEM_CHANGED_Y,
    DRAG_POINT_NW_CHANGED_X,
    DRAG_POINT_NW_CHANGED_y,
    N_HANDLER
};

typedef struct
{
    /* private members go here */
    guint id;
    gboolean is_dragged;
    MyDragPoint *drag_point[N_DRAG_POINTS];

    gulong handler[N_HANDLER];
} MySystemPrivate;


static void my_system_json_serializable (JsonSerializableIface * iface);

G_DEFINE_TYPE_WITH_CODE (MySystem, my_system, GOC_TYPE_WIDGET,
                         G_ADD_PRIVATE (MySystem)
                         G_IMPLEMENT_INTERFACE (JSON_TYPE_SERIALIZABLE,
                                                my_system_json_serializable))

     static JsonNode *_serialize_property (JsonSerializable * serializable,
                                           const gchar * name,
                                           const GValue * value,
                                           GParamSpec * pspec)
{
    JsonNode *json_node = NULL;

    g_return_val_if_fail (MY_IS_SYSTEM (serializable), FALSE);

    if (g_str_equal (name, "widget")
        || g_str_equal (name, "canvas")
        || g_str_equal (name, "parent")
        ) {
        return json_node;
    }
    else {
        /* Default serialization here (pixbufs excluded) */
        JsonSerializableIface *iface = NULL;

        iface = g_type_default_interface_peek (JSON_TYPE_SERIALIZABLE);
        json_node = iface->serialize_property (serializable,
                                               name, value, pspec);
    }

    return json_node;           /* NULL indicates default deserializer */
}

static gboolean
_deserialize_property (JsonSerializable * serializable,
                       const gchar * name,
                       GValue * value, GParamSpec * pspec, JsonNode * node)
{
    gboolean result = FALSE;

    g_return_val_if_fail (node != NULL, FALSE);

    JsonSerializableIface *iface = NULL;

    iface = g_type_default_interface_peek (JSON_TYPE_SERIALIZABLE);
    result = iface->deserialize_property (serializable,
                                          name, value, pspec, node);

    return result;
}

static void
my_system_json_serializable (JsonSerializableIface * iface)
{
    iface->serialize_property = _serialize_property;
    iface->deserialize_property = _deserialize_property;
}

static void
my_system_set_property (GObject * object,
                        guint property_id,
                        const GValue * value, GParamSpec * pspec)
{
    MySystem *self = MY_SYSTEM (object);
    MySystemPrivate *priv = my_system_get_instance_private (self);

    switch (property_id) {

        case PROP_ID:
            priv->id = g_value_get_uint (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_system_get_property (GObject * object,
                        guint property_id, GValue * value, GParamSpec * pspec)
{
    MySystem *self = MY_SYSTEM (object);

    MySystemPrivate *priv = my_system_get_instance_private (self);


    switch (property_id) {

        case PROP_ID:
            g_value_set_uint (value, priv->id);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

GQuark
my_system_error_quark (void)
{
    return g_quark_from_static_string ("my-system-error-quark");
}

MyAnchorType
calculate_anchor_of_dest (cairo_rectangle_t from, cairo_rectangle_t dest)
{
    gdouble dx, dy, alpha;
    MyAnchorType anchor;

    dx = (dest.x + (dest.width / 2.0)) - (from.x + (from.width / 2.0));
    dy = (dest.y + (dest.height / 2.0)) - (from.y + (from.height / 2.0));

    alpha = atan2 (dy, dx);

    /*g_print("from.x: %5.0f, from.y: %5.0f, dest.x: %5.0f, dest.y: %5.0f\n", from.x, from.y, dest.x, dest.y);*/
    /*g_print("from.width: %5.0f, from.height: %5.0f, dest.width: %5.0f, dest.height: %5.0f\n", from.width, from.height, dest.width, dest.height);*/
    /*g_print("dx: %5.0f, dy: %5.0f, alpha: %f\n", dx, dy, alpha * 180.0 / G_PI);*/

    anchor = MY_ANCHOR_NORTH;

    if (atan2 (-from.height / 2, from.width / 2) < alpha
        && alpha <= atan2 (from.height / 2, from.width / 2)) {
        anchor = MY_ANCHOR_WEST;
    }
    else if (atan2 (-from.height / 2, -from.width / 2) < alpha
             && alpha <= atan2 (-from.height / 2, from.width / 2)) {
        anchor = MY_ANCHOR_SOUTH;
    }
    else if (atan2 (from.height / 2, -from.width / 2) < alpha
             || alpha < atan2 (-from.height / 2, from.width / 2)) {
        anchor = MY_ANCHOR_EAST;
    }
    return anchor;
}

void
alloc_get_coordinate_of_anchor (cairo_rectangle_t alloc,
                                MyAnchorType anchor, gdouble * x, gdouble * y)
{
    if (anchor == MY_ANCHOR_WEST) {
        *x = alloc.x;
        *y = alloc.y + alloc.height / 2;
    }
    else if (anchor == MY_ANCHOR_SOUTH) {
        *x = alloc.x + alloc.width / 2;
        *y = alloc.y + alloc.height;
    }
    else if (anchor == MY_ANCHOR_NORTH) {
        *x = alloc.x + alloc.width / 2;
        *y = alloc.y;
    }
    else if (anchor == MY_ANCHOR_EAST) {
        *x = alloc.x + alloc.width;
        *y = alloc.y + alloc.height / 2;
    }
}

void
my_system_get_coordinate_of_anchor (MySystem * system, MyAnchorType anchor,
                                    gdouble * x, gdouble * y)
{
    cairo_rectangle_t alloc;

    g_return_if_fail (MY_IS_SYSTEM (system));

    my_system_get_allocation (system, &alloc);

    alloc_get_coordinate_of_anchor (alloc, anchor, x, y);
}

MyAnchorType
get_dynamic_coordinate_of_dest (cairo_rectangle_t from,
                                cairo_rectangle_t dest, gdouble * x,
                                gdouble * y)
{
    MyAnchorType anchor;

    anchor = calculate_anchor_of_dest (from, dest);

    alloc_get_coordinate_of_anchor (dest, anchor, x, y);

    return anchor;
}

void
my_system_get_dynamic_coordinates_of_arrow_at_anchor (MySystem * self,
                                                      MyAnchorType ta, guint m,
                                                      guint n, gdouble * x0,
                                                      gdouble * y0)
{
    my_system_get_coordinate_of_anchor (self, ta, x0, y0);

    if (n < 2)
        return;

    if (ta == MY_ANCHOR_WEST || ta == MY_ANCHOR_EAST) {
        *y0 = *y0 + 120.0 * ((gdouble) m / (n - 1) - 0.5);
    }
    else {
        *x0 = *x0 + 150.0 * ((gdouble) m / (n - 1) - 0.5);
    }
}

void
my_system_draw_energy_flow_distribute_arrows (MySystem * self,
                                              GocGroup * group_arrows)
{
    guint n[N_MY_ANCHORS], m[N_MY_ANCHORS];
    guint i;
    gdouble energy_quantity;
    GList *l;

    gint pa, sa, ta;

    MySystem *primary_system, *secondary_system;

    for (i = 0; i < N_MY_ANCHORS; i++)
        n[i] = m[i] = 0;

    /* distribute arrows if anchors are the same */
    for (l = group_arrows->children; l != NULL; l = l->next) {

        if (!MY_IS_FLOW_ARROW (l->data))
            continue;

        g_object_get (l->data, "primary-anchor", &pa, "secondary-anchor",
                      &sa, "primary-system", &primary_system,
                      "secondary-system", &secondary_system, NULL);

        if (primary_system == self)
            n[pa]++;
        else if (secondary_system == self)
            n[sa]++;

    }

    for (l = group_arrows->children; l != NULL; l = l->next) {

        gdouble x0, y0, x1, y1;
        MyFlowArrow *arrow;

        if (!MY_IS_FLOW_ARROW (l->data))
            continue;

        arrow = MY_FLOW_ARROW (l->data);

        g_object_get (arrow, "primary-anchor", &pa, "secondary-anchor",
                      &sa, "secondary-system", &secondary_system,
                      "primary-system", &primary_system, NULL);

        if (self == primary_system)
            ta = pa;
        else if (self == secondary_system) {
            ta = sa;
        }
        else
            continue;

        my_system_get_dynamic_coordinates_of_arrow_at_anchor (self, ta, m[ta],
                                                              n[ta], &x0, &y0);

        /* if arrow connects to environment */
        if (!MY_IS_SYSTEM (secondary_system)) {
            if (ta == MY_ANCHOR_WEST || ta == MY_ANCHOR_EAST)
                my_flow_arrow_set_coordinate (arrow, "y0", y0, "y1", y0, NULL);
            else
                my_flow_arrow_set_coordinate (arrow, "x0", x0, "x1", x0, NULL);
        }
        else {
            my_flow_arrow_set_coordinate (arrow, "y0", y0, "x0", x0, NULL);
        }

        m[ta]++;
    }
}

static void
my_system_class_init (MySystemClass * klass)
{
    GObjectClass *gobject_class;
    GocItemClass *gi_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gi_class = (GocItemClass *) klass;

    gobject_class->set_property = my_system_set_property;
    gobject_class->get_property = my_system_get_property;

    gobject_class->finalize = my_system_finalize;
    gobject_class->dispose = my_system_dispose;

    obj_properties[PROP_ID] =
        g_param_spec_uint ("id",
                           "id",
                           "unique identifier of system",
                           0, G_MAXUINT, 0,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);
}

void
my_system_get_allocation (MySystem * self, cairo_rectangle_t * alloc)
{
    g_object_get (self, "x", &alloc->x, "y", &alloc->y, "width", &alloc->width,
                  "height", &alloc->height, NULL);
}

static void
my_system_coordinates_changed (MySystem * self,
                               GParamSpec * pspec, gpointer data)
{
    GocGroup *group_arrows = NULL;
    MyCanvas *canvas;
    GList *l;

    MySystemClass *class = MY_SYSTEM_GET_CLASS (self);

    g_object_get (self, "canvas", &canvas, NULL);

    if (!MY_IS_CANVAS (canvas))
        return;

    group_arrows = canvas->group[GROUP_ARROWS];

    g_return_if_fail (GOC_IS_GROUP (group_arrows));

    for (l = group_arrows->children; l != NULL; l = l->next) {

        MySystem *secondary_system, *primary_system;
        MyAnchorType secondary_anchor, primary_anchor;
        cairo_rectangle_t primary_alloc, secondary_alloc;

        gdouble x0, x1, y0, y1, arrow_len;

        if (!MY_IS_FLOW_ARROW (l->data)) {
            continue;
        }

        g_object_get (l->data, "primary-system", &primary_system,
                      "secondary-system", &secondary_system, NULL);

        /* fetch next arrow if current arrow doesn't belong to system */
        if (primary_system != self && secondary_system != self) {
            continue;
        }

        g_object_get (l->data, "primary-anchor", &primary_anchor, NULL);

        /* draw arrow */

        my_system_get_allocation (primary_system, &primary_alloc);

        arrow_len = primary_alloc.width * 0.3;

        /* if arrow depicts transfer between primary and secondary system */
        if (MY_IS_SYSTEM (secondary_system)) {

            my_system_get_allocation (secondary_system, &secondary_alloc);

            secondary_anchor =
                get_dynamic_coordinate_of_dest
                (primary_alloc, secondary_alloc, &x1, &y1);

            primary_anchor =
                get_dynamic_coordinate_of_dest
                (secondary_alloc, primary_alloc, &x0, &y0);

            g_object_set (l->data, "primary-anchor", primary_anchor,
                          "secondary-anchor", secondary_anchor, NULL);
        }
        /* if arrow depicts transfer between primary system and the environment */
        else {
            /* if arrow depicts transfer to environment */
            alloc_get_coordinate_of_anchor (primary_alloc,
                                            primary_anchor, &x0, &y0);

            if (primary_anchor == MY_ANCHOR_WEST) {
                x1 = x0 - arrow_len;
                y1 = y0;
            }
            else if (primary_anchor == MY_ANCHOR_EAST) {
                x1 = x0 + arrow_len;
                y1 = y0;
            }
            else if (primary_anchor == MY_ANCHOR_SOUTH) {
                x1 = x0;
                y1 = y0 + arrow_len;
            }
            else if (primary_anchor == MY_ANCHOR_NORTH) {
                x1 = x0;
                y1 = y0 - arrow_len;
            }
        }

        my_flow_arrow_set_coordinate (MY_FLOW_ARROW (l->data), "x0", x0,
                                      "y0", y0, "x1", x1, "y1", y1, NULL);


        /*if (MY_IS_SYSTEM (primary_system)) {*/
            /*my_system_draw_energy_flow_distribute_arrows (primary_system,*/
                                                          /*group_arrows);*/
        /*}*/

        /*if (MY_IS_SYSTEM (secondary_system)) {*/
            /*my_system_draw_energy_flow_distribute_arrows (secondary_system,*/
                                                          /*group_arrows);*/
        /*}*/
    }
}

static void
my_system_drag_point_coordinate_changed (MySystem * self,
                                         GParamSpec * pspec,
                                         MyDragPoint * point)
{
    gdouble x_dp, y_dp, x_s, y_s, width, height;
    MySystemPrivate *priv = my_system_get_instance_private (self);

    g_object_get (point, "x", &x_dp, "y", &y_dp, NULL);
    g_object_get (self, "x", &x_s, "y", &y_s, "width", &width, "height",
                  &height, NULL);

    g_signal_handler_block (self, priv->handler[SYSTEM_CHANGED_X]);
    g_signal_handler_block (self, priv->handler[SYSTEM_CHANGED_Y]);

    g_object_set (self, "x", x_dp, "y", y_dp, "width", width + (x_s - x_dp),
                  "height", height + (y_s - y_dp), NULL);

    g_signal_handler_unblock (self, priv->handler[SYSTEM_CHANGED_X]);
    g_signal_handler_unblock (self, priv->handler[SYSTEM_CHANGED_Y]);
}

static void
my_system_coordinate_changed (MySystem * self,
                              GParamSpec * pspec, gpointer data)
{
    gdouble x_dp, y_dp, x_s, y_s, width, height;

    MySystemPrivate *priv = my_system_get_instance_private (self);

    g_object_get (self, "x", &x_s, "y", &y_s, "width", &width, "height",
                  &height, NULL);

    g_signal_handler_block (priv->drag_point[DRAG_POINT_NW],
                            priv->handler[DRAG_POINT_NW_CHANGED_X]);
    g_signal_handler_block (priv->drag_point[DRAG_POINT_NW],
                            priv->handler[DRAG_POINT_NW_CHANGED_y]);

    g_object_set (priv->drag_point[DRAG_POINT_NW], "x", x_s, "y", y_s, NULL);

    g_signal_handler_unblock (priv->drag_point[DRAG_POINT_NW],
                              priv->handler[DRAG_POINT_NW_CHANGED_X]);
    g_signal_handler_unblock (priv->drag_point[DRAG_POINT_NW],
                              priv->handler[DRAG_POINT_NW_CHANGED_y]);
}

static void
my_system_canvas_changed (MySystem * self, GParamSpec * pspec, gpointer data)
{
    MySystemPrivate *priv = my_system_get_instance_private (self);

    MyCanvas *canvas;
    GocGroup *group_dragpoints;

    gdouble x, y;

    g_return_if_fail (MY_IS_SYSTEM (self));

    priv->is_dragged = FALSE;

    g_object_get (self, "canvas", &canvas, NULL);

    if (!MY_IS_CANVAS (canvas)) {
        return;
    }

    g_object_unref (canvas);

    g_object_get (self, "x", &x, "y", &y, NULL);

    group_dragpoints = canvas->group[GROUP_SYSTEM_DRAGPOINTS];

    g_return_if_fail (GOC_IS_GROUP (group_dragpoints));

    priv->drag_point[DRAG_POINT_NW] = (MyDragPoint *)
        goc_item_new (group_dragpoints, MY_TYPE_DRAG_POINT, "x", x, "y", y,
                      "radius", 10.0, "linked-item", self, NULL);

    priv->handler[DRAG_POINT_NW_CHANGED_X] =
        g_signal_connect_swapped (priv->drag_point[DRAG_POINT_NW], "notify::x",
                                  G_CALLBACK
                                  (my_system_drag_point_coordinate_changed),
                                  self);
    priv->handler[DRAG_POINT_NW_CHANGED_y] =
        g_signal_connect_swapped (priv->drag_point[DRAG_POINT_NW], "notify::y",
                                  G_CALLBACK
                                  (my_system_drag_point_coordinate_changed),
                                  self);

    priv->handler[SYSTEM_CHANGED_X] = g_signal_connect (self, "notify::x",
                                                        G_CALLBACK
                                                        (my_system_coordinate_changed),
                                                        NULL);
    priv->handler[SYSTEM_CHANGED_Y] =
        g_signal_connect (self, "notify::y",
                          G_CALLBACK (my_system_coordinate_changed), NULL);
}

static void
my_system_init (MySystem * self)
{
    MySystemWidget *system_widget;

    MySystemPrivate *priv = my_system_get_instance_private (self);

    system_widget = g_object_new (MY_TYPE_SYSTEM_WIDGET, "system", self, NULL);

    g_object_bind_property (self, "id", system_widget, "id",
                            G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    goc_item_set (GOC_ITEM (self), "widget", system_widget, "width", 300.0,
                  "height", 250.0, NULL);

    g_signal_connect (self, "notify::x",
                      G_CALLBACK (my_system_coordinates_changed), NULL);

    g_signal_connect (self, "notify::y",
                      G_CALLBACK (my_system_coordinates_changed), NULL);

    g_signal_connect (self, "notify::canvas",
                      G_CALLBACK (my_system_canvas_changed), NULL);
}

static void
my_system_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_system_parent_class)->dispose (object);
}

static void
my_system_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_system_parent_class)->finalize (object);
}

/* begin public methods */
