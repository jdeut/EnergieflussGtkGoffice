#include "my-flowarrow.h"

#include <json-glib/json-glib.h>

enum
{
    PROP_0,
    /* property entries */
    PROP_LABEL_TEXT,
    PROP_ENERGY_QUANTITY,
    PROP_PRIMARY_SYSTEM,
    PROP_SECONDARY_SYSTEM,
    PROP_PRIMARY_ANCHOR,
    PROP_SECONDARY_ANCHOR,
    PROP_TRANSFER_TYPE,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

enum
{
    CANVAS_CHANGED_ENERGY_QUANTITY,
    CANVAS_CHANGED_LABEL_TEXT,
    CANVAS_CHANGED_TRANSFER_TYPE,
    CANVAS_CHANGED_X0,
    CANVAS_CHANGED_Y0,
    CANVAS_CHANGED_X1,
    CANVAS_CHANGED_Y1,
    N_HANDLER
};

/* 'private'/'protected' functions */
static void my_flow_arrow_class_init (MyFlowArrowClass * klass);
static void my_flow_arrow_init (MyFlowArrow * self);
static void my_flow_arrow_finalize (GObject *);
static void my_flow_arrow_dispose (GObject *);
static void my_flow_arrow_bind_to_drag_point (MyFlowArrow * self);
void my_flow_arrow_set_transfer_type (MyFlowArrow * self, guint transfer_type);
static void my_flow_arrow_coordinates_changed (MyFlowArrow * self,
                                               GParamSpec * pspec,
                                               gpointer data);
static void notify_label_text_changed (MyFlowArrow * self, GParamSpec * pspec,
                                       gpointer data);


typedef struct
{
    gboolean is_dragged;
    GOArrow *arrow;
    GocItem *label;
    MyDragPoint *drag_point;

    GtkWidget *eventbox;

    GBinding *bind_drag_point_to_x;
    GBinding *bind_drag_point_to_y;

    MySystem *primary_system, *secondary_system;
    gint primary_anchor, secondary_anchor;
    gdouble energy_quantity;
    gchar *label_text;
    guint transfer_type;

    gulong handler[N_HANDLER];

} MyFlowArrowPrivate;

static guint globully = 0;

GType
my_anchor_type_get_type (void)
{
    static GType etype = 0;

    if (etype == 0) {
        static GEnumValue const values[] = {
            {MY_ANCHOR_NORTH, "MY_ANCHOR_NORTH", "north"},
            {MY_ANCHOR_SOUTH, "MY_ANCHOR_SOUTH", "south"},
            {MY_ANCHOR_WEST, "MY_ANCHOR_WEST", "west"},
            {MY_ANCHOR_EAST, "MY_ANCHOR_EAST", "east"},
            {0, NULL, NULL}
        };

        etype =
            g_enum_register_static (g_intern_static_string ("MyAnchorType"),
                                    values);
    }
    return etype;
}

static void my_flow_arrow_json_serializable_init (JsonSerializableIface *
                                                  iface);

G_DEFINE_TYPE_WITH_CODE (MyFlowArrow, my_flow_arrow, GOC_TYPE_LINE,
                         G_ADD_PRIVATE (MyFlowArrow)
                         G_IMPLEMENT_INTERFACE (JSON_TYPE_SERIALIZABLE,
                                                my_flow_arrow_json_serializable_init))

     static JsonNode *_serialize_property (JsonSerializable * serializable,
                                           const gchar * name,
                                           const GValue * value,
                                           GParamSpec * pspec)
{
    JsonNode *json_node = NULL;

    g_return_val_if_fail (MY_IS_FLOW_ARROW (serializable), FALSE);

    if (g_str_equal (name, "style")
        || g_str_equal (name, "end-arrow")
        || g_str_equal (name, "start-arrow")
        || g_str_equal (name, "scale-line-width")
        || g_str_equal (name, "canvas")
        || g_str_equal (name, "parent")
        || g_str_equal (name, "primary-system")
        || g_str_equal (name, "secondary-system")
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
my_flow_arrow_json_serializable_init (JsonSerializableIface * iface)
{
    iface->serialize_property = _serialize_property;
    iface->deserialize_property = _deserialize_property;
}

static void
my_flow_arrow_set_property (GObject * object,
                            guint property_id,
                            const GValue * value, GParamSpec * pspec)
{
    MyFlowArrow *self = MY_FLOW_ARROW (object);
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    switch (property_id) {

        case PROP_LABEL_TEXT:
            g_free (priv->label_text);
            priv->label_text = g_value_dup_string (value);
            break;

        case PROP_PRIMARY_SYSTEM:{
                MySystem *primary_system =
                    MY_SYSTEM (g_value_get_object (value));
                my_flow_arrow_set_linked_system (self, primary_system);
                return;
            }

        case PROP_SECONDARY_SYSTEM:
            priv->secondary_system = g_value_get_object (value);
            break;

        case PROP_ENERGY_QUANTITY:
            priv->energy_quantity = g_value_get_double (value);
            break;

        case PROP_PRIMARY_ANCHOR:
            priv->primary_anchor = (MyAnchorType) g_value_get_enum (value);
            break;

        case PROP_SECONDARY_ANCHOR:
            priv->secondary_anchor = (MyAnchorType) g_value_get_enum (value);
            break;

        case PROP_TRANSFER_TYPE:
            my_flow_arrow_set_transfer_type (self, g_value_get_uint (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_flow_arrow_get_property (GObject * object,
                            guint property_id,
                            GValue * value, GParamSpec * pspec)
{
    MyFlowArrow *self = MY_FLOW_ARROW (object);
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    switch (property_id) {

        case PROP_LABEL_TEXT:
            g_value_set_string (value, priv->label_text);
            break;

        case PROP_PRIMARY_SYSTEM:
            g_value_set_object (value, priv->primary_system);
            break;

        case PROP_SECONDARY_SYSTEM:
            g_value_set_object (value, priv->secondary_system);
            break;

        case PROP_ENERGY_QUANTITY:
            g_value_set_double (value, priv->energy_quantity);
            break;

        case PROP_PRIMARY_ANCHOR:
            g_value_set_enum (value, priv->primary_anchor);
            break;

        case PROP_SECONDARY_ANCHOR:
            g_value_set_enum (value, priv->secondary_anchor);
            break;

        case PROP_TRANSFER_TYPE:
            g_value_set_uint (value, priv->transfer_type);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void
my_flow_arrow_set_transfer_type (MyFlowArrow * self, guint transfer_type)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    GOStyle *style;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    priv->transfer_type = transfer_type;

    g_object_get (self, "style", &style, NULL);

    if (transfer_type == MY_TRANSFER_WORK) {
        style->line.color = GO_COLOR_FROM_RGBA (100, 200, 0, 255);
    }
    else if (transfer_type == MY_TRANSFER_MASS) {
        style->line.color = GO_COLOR_FROM_RGBA (0, 0, 0, 255);
    }
    else if (transfer_type == MY_TRANSFER_HEAT) {
        style->line.color = GO_COLOR_FROM_RGBA (255, 0, 0, 255);
    }
    else if (transfer_type == MY_TRANSFER_RADIATION) {
        style->line.color = GO_COLOR_FROM_RGBA (220, 220, 0, 255);
    }
    else {
    }
}


static gboolean
my_flow_arrow_button_pressed (GocItem * item, int button, double x, double y)
{

    GtkWidget *toplevel;

    MyFlowArrow *self = MY_FLOW_ARROW (item);
    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    parent_class->button_pressed (GOC_ITEM (self), button, x, y);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (item->canvas));

    if (gtk_widget_is_toplevel (toplevel)) {
        dialog_property_editor (G_OBJECT (item), "test", GTK_WINDOW (toplevel));
    }

    g_print ("button pressed...\n");

    return FALSE;
}

static gboolean
my_flow_arrow_leave_notify (GocItem * item, double x, double y)
{

    MyFlowArrow *self = MY_FLOW_ARROW (item);
    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    parent_class->leave_notify (GOC_ITEM (self), x, y);

    g_print ("leave arrow...\n");

    return FALSE;
}

static gboolean
my_flow_arrow_enter_notify (GocItem * item, double x, double y)
{

    MyFlowArrow *self = MY_FLOW_ARROW (item);
    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    parent_class->enter_notify (GOC_ITEM (self), x, y);

    g_print ("enter arrow...\n");

    return FALSE;
}

GQuark
my_flow_arrow_error_quark (void)
{
    return g_quark_from_static_string ("my-flow-arrow-error-quark");
}

void
my_flow_arrow_draw (GocItem const *item, cairo_t * cr)
{
    MyFlowArrow *self = MY_FLOW_ARROW (item);
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);
    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (self);

    GocItemClass *parent_class = g_type_class_peek_parent (class);

    /* chaining up */
    parent_class->draw (GOC_ITEM (self), cr);

    /*goc_item_invalidate (GOC_ITEM (self)); */
}

static void
my_flow_arrow_init_style (G_GNUC_UNUSED GocStyledItem * item, GOStyle * style)
{
    MyFlowArrow *self = MY_FLOW_ARROW (item);
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    style->interesting_fields = GO_STYLE_LINE;

    if (style->line.auto_dash)
        style->line.dash_type = GO_LINE_SOLID;
    if (style->line.auto_fore)
        style->line.fore = 0;

    my_flow_arrow_set_transfer_type (self, priv->transfer_type);

}

void
my_flow_arrow_realize (GocItem * item)
{

    MyFlowArrow *self = MY_FLOW_ARROW (item);

    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (item);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    /* chaining up */
    parent_class->realize (item);

    /*my_flow_arrow_bind_to_drag_point(self); */
}

static void
my_flow_arrow_class_init (MyFlowArrowClass * klass)
{
    GObjectClass *gobject_class;
    GocItemClass *gi_class;
    GocStyledItemClass *gsi_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gi_class = (GocItemClass *) klass;
    gsi_class = (GocStyledItemClass *) klass;

    gobject_class->finalize = my_flow_arrow_finalize;
    gobject_class->dispose = my_flow_arrow_dispose;
    gobject_class->set_property = my_flow_arrow_set_property;
    gobject_class->get_property = my_flow_arrow_get_property;

    obj_properties[PROP_LABEL_TEXT] =
        g_param_spec_string ("label-text",
                             "label-text",
                             "Text of the arrow label.",
                             NULL, G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    obj_properties[PROP_ENERGY_QUANTITY] =
        g_param_spec_double ("energy-quantity",
                             "energy-quantity",
                             "The energy quantity that is transfered. E > 0 means energy flows in the system an E < 0 means energy flows out of the system.",
                             -G_MAXDOUBLE, G_MAXDOUBLE, -10,
                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    obj_properties[PROP_PRIMARY_SYSTEM] =
        g_param_spec_object ("primary-system",
                             "primary system",
                             "A pointer to the system from where the energy transport is seen",
                             MY_TYPE_SYSTEM, G_PARAM_READWRITE);

    obj_properties[PROP_SECONDARY_SYSTEM] =
        g_param_spec_object ("secondary-system",
                             "secondary system",
                             "A pointer to the secondary system where the arrow is connected to",
                             MY_TYPE_SYSTEM, G_PARAM_READWRITE);

    obj_properties[PROP_PRIMARY_ANCHOR] =
        g_param_spec_enum ("primary-anchor",
                           "primary anchor",
                           "Determines on which side of the primary-system the arrow should snap",
                           MY_TYPE_ANCHOR_TYPE, MY_ANCHOR_EAST,
                           G_PARAM_READWRITE);

    obj_properties[PROP_SECONDARY_ANCHOR] =
        g_param_spec_enum ("secondary-anchor",
                           "secondary-anchor",
                           "Determines on which side of the primary-system the arrow should snap",
                           MY_TYPE_ANCHOR_TYPE, MY_ANCHOR_EAST,
                           G_PARAM_READWRITE);

    obj_properties[PROP_TRANSFER_TYPE] =
        g_param_spec_uint ("transfer-type",
                           "transfer-type",
                           "transfer type",
                           0, G_MAXUINT, 0,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);

    /*gi_class->draw = my_flow_arrow_draw; */
    gi_class->enter_notify = my_flow_arrow_enter_notify;
    gi_class->leave_notify = my_flow_arrow_leave_notify;
    gi_class->button_pressed = my_flow_arrow_button_pressed;
    /*gi_class->realize = my_flow_arrow_realize; */

    gsi_class->init_style = my_flow_arrow_init_style;
}

void
my_flow_arrow_set_coordinate (MyFlowArrow * self, const gchar * first_arg_name,
                              ...)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);
    va_list args;

    if (my_flow_arrow_is_dragged (self))
        return;

    va_start (args, first_arg_name);
    g_object_set_valist (G_OBJECT (self), first_arg_name, args);
    va_end (args);
}

static void
my_flow_arrow_bind_to_drag_point (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);
    gchar *x_str, *y_str;

    g_return_if_fail (MY_IS_DRAG_POINT (priv->drag_point));

    /* if energy_quantity < 0.0 than arrow tip ends on calculated anchor point on the secondary_system */
    if (priv->energy_quantity < 0.0) {
        x_str = g_strdup_printf ("x1");
        y_str = g_strdup_printf ("y1");
    }
    /* if energy_quantity > 0 than arrow tip ends on calculated anchor point on the primary_system,
     * so that the tail of the arrow is in the center of the secondary_system */
    else {
        x_str = g_strdup_printf ("x0");
        y_str = g_strdup_printf ("y0");
    }

    if (G_IS_OBJECT (priv->bind_drag_point_to_x))
        g_object_unref (priv->bind_drag_point_to_x);

    if (G_IS_OBJECT (priv->bind_drag_point_to_y))
        g_object_unref (priv->bind_drag_point_to_y);

    priv->bind_drag_point_to_x =
        g_object_bind_property (self, x_str, priv->drag_point, "x",
                                G_BINDING_BIDIRECTIONAL |
                                G_BINDING_SYNC_CREATE);

    priv->bind_drag_point_to_y =
        g_object_bind_property (self, y_str, priv->drag_point, "y",
                                G_BINDING_BIDIRECTIONAL |
                                G_BINDING_SYNC_CREATE);
}

static void
my_flow_arrow_energy_quantity_changed (MyFlowArrow * self,
                                       GParamSpec * pspec, gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    GOStyle *style;

    g_object_get (G_OBJECT (self), "style", &style, NULL);

    style->line.width = ABS (priv->energy_quantity);

    my_flow_arrow_bind_to_drag_point (self);
}

static void
notify_label_text_changed (MyFlowArrow * self, GParamSpec * pspec,
                           gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    if (GOC_IS_ITEM (priv->label)) {
        goc_item_destroy (priv->label);
        priv->label = NULL;
    }

    MyCanvas *canvas;

    GocGroup *group_labels = NULL;

    g_object_get (self, "canvas", &canvas, NULL);

    group_labels = canvas->group[GROUP_LABELS];

    if (priv->label_text != NULL) {

        if (!GOC_IS_ITEM (priv->label)) {

            GError *err = NULL;
            gchar *text;
            GtkWidget *widget;
            GtkStyleContext *context;

            priv->eventbox = gtk_event_box_new ();
            context = gtk_widget_get_style_context (priv->eventbox);
            gtk_style_context_add_class (context, "test");
            widget = gtk_label_new (priv->label_text);
            gtk_container_add (GTK_CONTAINER (priv->eventbox), widget);
            gtk_widget_set_visible (priv->eventbox, TRUE);
            gtk_widget_set_visible (widget, TRUE);

            priv->label =
                goc_item_new (canvas->group[GROUP_LABELS], GOC_TYPE_WIDGET,
                              "widget", priv->eventbox, NULL);

            goc_item_set (priv->label, "width", 100.0, "height", 50.0, NULL);

            gtk_widget_show_all (priv->eventbox);

            my_flow_arrow_coordinates_changed (self, NULL, NULL);
        }
    }
}

static void
my_flow_arrow_change_anchor_while_dragging (MyFlowArrow * self,
                                            GParamSpec * pspec, gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    GtkAllocation from, to;
    gdouble x, y;
    MyAnchorType anchor;

    g_return_if_fail (MY_IS_SYSTEM (priv->primary_system));

    if (!my_flow_arrow_is_dragged (self))
        return;

    if (priv->energy_quantity >= 0)
        g_object_get (self, "x0", &x, "y0", &y, NULL);
    else
        g_object_get (self, "x1", &x, "y1", &y, NULL);

    from.x = x;
    from.y = y;

    gtk_widget_get_allocation (GOC_WIDGET (priv->primary_system)->ofbox, &to);

    anchor = calculate_anchor (from, to);

    my_system_get_coordinate_of_anchor (priv->primary_system, anchor, &x, &y);

    g_object_set (self, "primary-anchor", anchor, NULL);

    g_signal_handlers_block_by_func (self,
                                     my_flow_arrow_change_anchor_while_dragging,
                                     NULL);

    if (priv->energy_quantity >= 0)
        g_object_set (self, "x1", x, "y1", y, NULL);
    else
        g_object_set (self, "x0", x, "y0", y, NULL);

    g_signal_handlers_unblock_by_func (self,
                                       my_flow_arrow_change_anchor_while_dragging,
                                       NULL);
}

static void
my_flow_arrow_transfer_type_changed (MyFlowArrow * self,
                                     GParamSpec * pspec, gpointer data)
{
    g_print ("transfer type changed\n");
}

static void
my_flow_arrow_canvas_changed (MyFlowArrow * self,
                              GParamSpec * pspec, gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    MyCanvas *canvas;
    GocGroup *group_dragpoints;
    guint i;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    priv->is_dragged = FALSE;

    if (G_IS_OBJECT (priv->bind_drag_point_to_x))
        g_object_unref (priv->bind_drag_point_to_x);
    if (G_IS_OBJECT (priv->bind_drag_point_to_y))
        g_object_unref (priv->bind_drag_point_to_y);

    priv->bind_drag_point_to_y = NULL;
    priv->bind_drag_point_to_x = NULL;

    g_object_get (self, "canvas", &canvas, NULL);

    if (!MY_IS_CANVAS (canvas)) {
        return;
    }

    g_object_unref (canvas);

    group_dragpoints = canvas->group[GROUP_DRAGPOINTS];

    g_return_if_fail (GOC_IS_GROUP (group_dragpoints));

    for (i = 0; i < N_HANDLER; i++) {
        if (priv->handler[i] != 0) {
            g_signal_handler_disconnect (self, priv->handler[i]);
        }
    }

    priv->drag_point = (MyDragPoint *)
        goc_item_new (group_dragpoints, MY_TYPE_DRAG_POINT, "radius", 10.0,
                      "linked-item", self, NULL);

    priv->handler[CANVAS_CHANGED_ENERGY_QUANTITY] =
        g_signal_connect (self, "notify::energy-quantity",
                          G_CALLBACK (my_flow_arrow_energy_quantity_changed),
                          NULL);

    g_object_notify (G_OBJECT (self), "energy-quantity");

    priv->handler[CANVAS_CHANGED_LABEL_TEXT] =
        g_signal_connect (self, "notify::label-text",
                          G_CALLBACK (notify_label_text_changed), NULL);

    priv->handler[CANVAS_CHANGED_X0] =
        g_signal_connect (self, "notify::x0",
                          G_CALLBACK
                          (my_flow_arrow_change_anchor_while_dragging), NULL);

    priv->handler[CANVAS_CHANGED_X1] =
        g_signal_connect (self, "notify::x1",
                          G_CALLBACK
                          (my_flow_arrow_change_anchor_while_dragging), NULL);

    priv->handler[CANVAS_CHANGED_Y0] =
        g_signal_connect (self, "notify::y0",
                          G_CALLBACK
                          (my_flow_arrow_change_anchor_while_dragging), NULL);

    priv->handler[CANVAS_CHANGED_Y1] =
        g_signal_connect (self, "notify::y1",
                          G_CALLBACK
                          (my_flow_arrow_change_anchor_while_dragging), NULL);

    priv->handler[CANVAS_CHANGED_TRANSFER_TYPE] =
        g_signal_connect (self, "notify::transfer-type",
                          G_CALLBACK (my_flow_arrow_transfer_type_changed),
                          NULL);
}


static void
my_flow_arrow_coordinates_changed (MyFlowArrow * self,
                                   GParamSpec * pspec, gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gdouble x0, x1, y0, y1;
    gdouble angle;
    cairo_matrix_t matrix;

    g_object_get (self, "x0", &x0, "y0", &y0, "x1", &x1, "y1", &y1, NULL);

    if (!GOC_IS_WIDGET (priv->label))
        return;

    angle = atan2 (y1 - y0, x1 - x0);

    if (angle < 0) {
        angle += 2 * M_PI;
    }

    goc_item_set (priv->label, "x", x0 + (x1 - x0) / 2, "y",
                  y0 + (y1 - y0) / 2, NULL);

    cairo_matrix_init_identity (&matrix);
    cairo_matrix_translate (&matrix, priv->energy_quantity / 2 * sin (angle),
                            -priv->energy_quantity / 2 * cos (angle));

    goc_item_set_transform (priv->label, &matrix);

    goc_item_invalidate (priv->label);
}

static void
my_flow_arrow_init (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);
    GOStyle *style;
    guint i;

    priv->is_dragged = FALSE;
    priv->arrow = g_new0 (GOArrow, 1);
    priv->transfer_type = MY_TRANSFER_WORK;

    for (i = 0; i < N_HANDLER; i++) {
        priv->handler[i] = 0;
    }

    g_signal_connect (self, "notify::canvas",
                      G_CALLBACK (my_flow_arrow_canvas_changed), NULL);

    g_signal_connect (self, "notify::x0",
                      G_CALLBACK (my_flow_arrow_coordinates_changed), NULL);
    g_signal_connect (self, "notify::x1",
                      G_CALLBACK (my_flow_arrow_coordinates_changed), NULL);
    g_signal_connect (self, "notify::y0",
                      G_CALLBACK (my_flow_arrow_coordinates_changed), NULL);
    g_signal_connect (self, "notify::y1",
                      G_CALLBACK (my_flow_arrow_coordinates_changed), NULL);

    go_arrow_init_kite (priv->arrow, 40, 40, 6);

    g_object_set (self, "end-arrow", priv->arrow, NULL);

}

static void
my_flow_arrow_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_flow_arrow_parent_class)->dispose (object);
}

static void
my_flow_arrow_finalize (GObject * object)
{
    MyFlowArrowPrivate *priv =
        my_flow_arrow_get_instance_private (MY_FLOW_ARROW (object));

    MyFlowArrow *self = MY_FLOW_ARROW (object);

    if (GOC_IS_ITEM (priv->label)) {
        goc_item_destroy (GOC_ITEM (priv->label));
    }

    if (MY_IS_DRAG_POINT (priv->drag_point)) {
        goc_item_destroy (GOC_ITEM (priv->drag_point));
    }

    g_free (priv->arrow);

    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_flow_arrow_parent_class)->finalize (object);
}

/* public methods */

void
my_flow_arrow_begin_dragging (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    priv->is_dragged = TRUE;
}

gboolean
my_flow_arrow_is_dragged (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    return priv->is_dragged;
}

void
my_flow_arrow_end_dragging (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    priv->is_dragged = FALSE;

    goc_item_invalidate (GOC_ITEM (self));

    g_object_notify(G_OBJECT(priv->primary_system), "x");
}

MyDragPoint *
my_flow_arrow_get_drag_point (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    return priv->drag_point;
}

void
my_flow_arrow_show_drag_points (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    if (MY_IS_DRAG_POINT (priv->drag_point)) {
        goc_item_show (GOC_ITEM (priv->drag_point));
    }
}

void
my_flow_arrow_hide_drag_points (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    if (MY_IS_DRAG_POINT (priv->drag_point)) {
        goc_item_hide (GOC_ITEM (priv->drag_point));
    }
}

MySystem *
my_flow_arrow_get_linked_system (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    return priv->primary_system;
}

void
my_flow_arrow_set_linked_system (MyFlowArrow * self, MySystem * system)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_SYSTEM (system));
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    priv->primary_system = system;
}
