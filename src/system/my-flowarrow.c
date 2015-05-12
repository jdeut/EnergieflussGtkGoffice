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
    PROP_UNIT,
    PROP_TRANSFER_TYPE,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

enum
{
    POPOVER_BINDING_ENERGY_QUANTITY,
    POPOVER_BINDING_LABEL_TEXT,
    N_POPOVER_BINDING
};

enum
{
    POPOVER_HANDLER_TRANSFER_TYPE_CHANGED,
    POPOVER_HANDLER_UNIT_CHANGED,
    POPOVER_HANDLER_POPOVER_CLOSED,
    N_POPOVER_HANDLER
};

enum
{
    CANVAS_CHANGED_ENERGY_QUANTITY,
    CANVAS_CHANGED_ENERGY_QUANTITY_UPDATE_INTENSITY_BOX,
    CANVAS_CHANGED_PRIMARY_SYSTEM_UPDATE_INTENSITY_BOX,
    CANVAS_CHANGED_SECONDARY_SYSTEM_UPDATE_INTENSITY_BOX,
    CANVAS_CHANGED_TRANSFER_TYPE,
    CANVAS_CHANGED_LABEL_TEXT,
    PRIMARY_SYSTEM_CHANGED,
    SECONDARY_SYSTEM_CHANGED,
    FLOW_ARROW_CHANGED_X1,
    FLOW_ARROW_CHANGED_Y1,
    DRAG_POINT_NW_CHANGED_X,
    DRAG_POINT_NW_CHANGED_Y,
    APP_WINDOW_CHANGED_METRIC_PREFIX,
    ACTION_SHOW_ENERGY_AMOUNT_STATE_CHANGED,
    N_HANDLER
};

/* 'private'/'protected' functions */
static void my_flow_arrow_class_init (MyFlowArrowClass * klass);
static void my_flow_arrow_init (MyFlowArrow * self);
static void my_flow_arrow_finalize (GObject *);
static void my_flow_arrow_dispose (GObject *);
void my_flow_arrow_set_transfer_type (MyFlowArrow * self, guint transfer_type);
static void my_flow_arrow_update_label (MyFlowArrow * self,
                                        GParamSpec * pspec, gpointer data);
static void my_flow_arrow_label_text_changed (MyFlowArrow * self,
                                              GParamSpec * pspec,
                                              gpointer data);
static gboolean my_flow_arrow_is_energy_amount_visible (MyFlowArrow * self);
void my_flow_arrow_update (MyFlowArrow * self);
static void
my_flow_arrow_sync_coordinate_with_drag_point_coordinate (MyFlowArrow * self,
                                                          GParamSpec * pspec,
                                                          gpointer data);
gdouble my_flow_arrow_get_preferred_width (MyFlowArrow * self);
gdouble my_flow_arrow_get_unit_factor (MyFlowArrow * self);
static void my_flow_arrow_primary_system_changed (MyFlowArrow * self);
static void my_flow_arrow_secondary_system_changed (MyFlowArrow * self);
void _primary_system_finalized (gpointer data, GObject * old);
void _secondary_system_finalized (gpointer data, GObject * old);


typedef struct
{
    gboolean is_dragged;
    GOArrow *arrow_start;
    GOArrow *arrow_end;
    GocItem *label;
    GtkCssProvider *provider;

    MyDragPoint *drag_point;

    GtkWidget *eventbox;

    MySystem *primary_system, *secondary_system;
    gint primary_anchor, secondary_anchor;

    gdouble energy_quantity;
    gdouble prefix_factor;

    gchar *label_text;

    guint transfer_type;
    guint unit;

    gulong handler[N_HANDLER];
    gpointer handler_instance[N_HANDLER];

    gulong popover_handler[N_POPOVER_HANDLER];
    gpointer popover_handler_instance[N_POPOVER_HANDLER];
    GBinding *popover_binding[N_POPOVER_BINDING];

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

        case PROP_PRIMARY_SYSTEM:

            if (MY_IS_SYSTEM (priv->primary_system))
                g_object_weak_unref (G_OBJECT (priv->primary_system),
                                     _primary_system_finalized, self);

            priv->primary_system = MY_SYSTEM (g_value_get_object (value));

            g_object_weak_ref (G_OBJECT (priv->primary_system),
                               _primary_system_finalized, self);
            break;

        case PROP_SECONDARY_SYSTEM:
            if (MY_IS_SYSTEM (priv->secondary_system))
                g_object_weak_unref (G_OBJECT (priv->secondary_system),
                                     _secondary_system_finalized, self);

            priv->secondary_system = MY_SYSTEM (g_value_get_object (value));

            if (G_IS_OBJECT (priv->secondary_system))
                g_object_weak_ref (G_OBJECT (priv->secondary_system),
                                   _secondary_system_finalized, self);
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

        case PROP_UNIT:
            priv->unit = g_value_get_uint (value);
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

        case PROP_UNIT:
            g_value_set_uint (value, priv->unit);
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
my_flow_arrow_transfer_type_changed (MyFlowArrow * self,
                                     GParamSpec * pspec, gpointer data)
{

    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    GOStyle *style;
    GtkStyleContext *context;
    GdkRGBA rgba;
    gchar *str;

    g_object_get (self, "style", &style, NULL);

    if (!GTK_IS_WIDGET (priv->eventbox)) {
        return;
    }

    go_color_to_gdk_rgba (style->line.color, &rgba);

    context = gtk_widget_get_style_context (priv->eventbox);

    str =
        g_strdup_printf
        (".label { background-color: rgba(%.0f, %.0f, %.0f, 1); }",
         255 * 1.2 * rgba.red, 255 * 1.2 * rgba.green, 255 * 1.2 * rgba.blue);

    gtk_css_provider_load_from_data (GTK_CSS_PROVIDER (priv->provider), str, -1,
                                     NULL);

    g_free (str);

    gtk_style_context_add_provider (context,
                                    GTK_STYLE_PROVIDER (priv->provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);

    goc_item_invalidate (GOC_ITEM (self));
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
        style->line.color = GO_COLOR_FROM_RGBA (230, 20, 20, 255);
    }
    else if (transfer_type == MY_TRANSFER_RADIATION) {
        style->line.color = GO_COLOR_FROM_RGBA (220, 220, 0, 255);
    }
}

void
settings_adjust_step_increment_to_unit (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    GtkWindow *window;
    gdouble unit_factor;
    FlowArrowSettings fas;

    window = my_application_get_active_window ();

    fas = my_window_get_flow_arrow_settings (MY_WINDOW (window));

    unit_factor = my_flow_arrow_get_unit_factor (self);

    gtk_adjustment_set_step_increment (fas.adj, 35.0 / unit_factor);
}

void
my_flow_arrow_combo_unit_row_changed (MyFlowArrow * self, GtkWidget * box)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    guint nr;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    gtk_combo_box_get_active_iter (GTK_COMBO_BOX (box), &iter);
    model = gtk_combo_box_get_model (GTK_COMBO_BOX (box));

    gtk_tree_model_get (model, &iter, 1, &nr, -1);

    g_object_set (self, "energy-unit", nr, NULL);

    g_object_notify (G_OBJECT (self), "energy-quantity");

    settings_adjust_step_increment_to_unit (self);
}

void
my_flow_arrow_combo_transfer_type_row_changed (MyFlowArrow * self,
                                               GtkWidget * box)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    guint nr;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    gtk_combo_box_get_active_iter (GTK_COMBO_BOX (box), &iter);
    model = gtk_combo_box_get_model (GTK_COMBO_BOX (box));

    gtk_tree_model_get (model, &iter, 1, &nr, -1);

    g_object_set (self, "transfer-type", nr, NULL);
}

gdouble
my_flow_arrow_get_unit_factor (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    switch (priv->unit) {
        case UNIT_JOULE:
            return 1.0;
        case UNIT_WATTHOUR:
            return 3600.0;
        case UNIT_CAL:
            return 4.18;
        default:
            break;
    }
}

gdouble
my_flow_arrow_energy_quantity_transform_to_metric_and_unit (MyFlowArrow * self)
{

    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    return priv->energy_quantity / (priv->prefix_factor *
                                    my_flow_arrow_get_unit_factor (self));
}

gchar *
my_flow_arrow_unit_get_name (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gchar *str;

    switch (priv->unit) {
        case UNIT_JOULE:
            str = g_strdup_printf ("%s", "J");
            break;
        case UNIT_WATTHOUR:
            str = g_strdup_printf ("%s", "Wh");
            break;
        case UNIT_CAL:
            str = g_strdup_printf ("%s", "cal");
            break;
    }

    return str;
}

gchar *
my_flow_arrow_prefix_get_name (MyFlowArrow * self)
{

    GtkWidget *activewindow;
    gchar *str;
    guint factor;


    activewindow = (GtkWidget *) my_application_get_active_window ();
    factor = my_window_get_metric_prefix (MY_WINDOW (activewindow));

    switch (factor) {
        case FACTOR_MILLI:
            str = g_strdup_printf ("%s", "m");
            break;
        case FACTOR_ONE:
            str = g_strdup_printf ("%s", "");
            break;
        case FACTOR_KILO:
            str = g_strdup_printf ("%s", "k");
            break;
        case FACTOR_MEGA:
            str = g_strdup_printf ("%s", "M");
            break;
        case FACTOR_GIGA:
            str = g_strdup_printf ("%s", "G");
            break;
    }

    return str;
}

gboolean
binding_energy_quantitiy_transform_to (GBinding * binding,
                                       const GValue * from_value,
                                       GValue * to_value, MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gdouble energy_quantity;

    energy_quantity = g_value_get_double (from_value);

    energy_quantity /= priv->prefix_factor *
        my_flow_arrow_get_unit_factor (self);

    g_value_set_double (to_value, energy_quantity);

    return TRUE;
}

gboolean
binding_energy_quantitiy_transform_from (GBinding * binding,
                                         const GValue * from_value,
                                         GValue * to_value, MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gdouble energy_quantity;

    energy_quantity = g_value_get_double (from_value);

    energy_quantity *=
        priv->prefix_factor * my_flow_arrow_get_unit_factor (self);

    g_value_set_double (to_value, energy_quantity);

    return TRUE;
}

static void
my_flow_arrow_popover_sync_controls_with_props (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    FlowArrowSettings fas;

    guint transfer_type, metric_prefix, unit, n;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean valid;

    GtkWidget *activewindow;

    activewindow = (GtkWidget *) my_application_get_active_window ();

    fas = my_window_get_flow_arrow_settings (MY_WINDOW (activewindow));

    g_object_get (self, "transfer-type", &transfer_type, NULL);
    g_object_get (self, "energy-unit", &unit, NULL);
    g_object_get (activewindow, "metric-prefix", &metric_prefix, NULL);

    model = gtk_combo_box_get_model (fas.transfer_type);

    valid = gtk_tree_model_get_iter_first (model, &iter);

    /* sync active iter of combobox with current transfer-type */
    while (valid) {
        gtk_tree_model_get (model, &iter, 1, &n, -1);

        if (n == transfer_type) {
            gtk_combo_box_set_active_iter (fas.transfer_type, &iter);
            break;
        }
        valid = gtk_tree_model_iter_next (model, &iter);
    }

    model = gtk_combo_box_get_model (fas.prefix);

    valid = gtk_tree_model_get_iter_first (model, &iter);

    /* sync active iter of combobox with current metric-prefix */
    while (valid) {
        gtk_tree_model_get (model, &iter, 1, &n, -1);

        if (n == metric_prefix) {
            gtk_combo_box_set_active_iter (fas.prefix, &iter);
            break;
        }
        valid = gtk_tree_model_iter_next (model, &iter);
    }

    model = gtk_combo_box_get_model (fas.unit);

    valid = gtk_tree_model_get_iter_first (model, &iter);

    /* sync active iter of combobox with current unit */
    while (valid) {
        gtk_tree_model_get (model, &iter, 1, &n, -1);

        if (n == unit) {
            gtk_combo_box_set_active_iter (fas.unit, &iter);
            break;
        }
        valid = gtk_tree_model_iter_next (model, &iter);
    }

    settings_adjust_step_increment_to_unit (self);

    priv->popover_handler_instance[POPOVER_HANDLER_TRANSFER_TYPE_CHANGED] =
        fas.transfer_type;

    priv->popover_handler[POPOVER_HANDLER_TRANSFER_TYPE_CHANGED] =
        g_signal_connect_swapped (fas.transfer_type, "changed",
                                  G_CALLBACK
                                  (my_flow_arrow_combo_transfer_type_row_changed),
                                  self);

    priv->popover_handler_instance[POPOVER_HANDLER_UNIT_CHANGED] = fas.unit;

    priv->popover_handler[POPOVER_HANDLER_UNIT_CHANGED] =
        g_signal_connect_swapped (fas.unit, "changed",
                                  G_CALLBACK
                                  (my_flow_arrow_combo_unit_row_changed), self);

    priv->popover_binding[POPOVER_BINDING_LABEL_TEXT] =
        g_object_bind_property (self, "label-text", fas.label, "text",
                                G_BINDING_BIDIRECTIONAL |
                                G_BINDING_SYNC_CREATE);

    priv->popover_binding[POPOVER_BINDING_ENERGY_QUANTITY] =
        g_object_bind_property_full (self, "energy-quantity", fas.adj,
                                     "value",
                                     G_BINDING_BIDIRECTIONAL |
                                     G_BINDING_SYNC_CREATE,
                                     (GBindingTransformFunc)
                                     binding_energy_quantitiy_transform_to,
                                     (GBindingTransformFunc)
                                     binding_energy_quantitiy_transform_from,
                                     self, NULL);
}

void
my_flow_arrow_popover_closed (MyFlowArrow * self, GtkPopover * popover)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gint i;

    for (i = 0; i < N_POPOVER_BINDING; i++)
        g_binding_unbind (priv->popover_binding[i]);

    for (i = 0; i < N_POPOVER_HANDLER; i++)
        g_signal_handler_disconnect (priv->popover_handler_instance[i],
                                     priv->popover_handler[i]);
}

static void
my_flow_arrow_popover_show (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    FlowArrowSettings fas;
    GtkWidget *activewindow;
    GdkRectangle rect;
    GdkDeviceManager *device_manager;
    GdkDisplay *display;
    GdkWindow *window;
    GdkDevice *device;
    gint x, y;

    activewindow = (GtkWidget *) my_application_get_active_window ();

    window = gtk_widget_get_window (GTK_WIDGET (GOC_ITEM (self)->canvas));
    display = gdk_window_get_display (window);
    device_manager = gdk_display_get_device_manager (display);
    device = gdk_device_manager_get_client_pointer (device_manager);

    gdk_window_get_device_position (window, device, &x, &y, NULL);

    rect.x = x;
    rect.y = y;
    rect.width = 2;
    rect.height = 2;

    fas = my_window_get_flow_arrow_settings (MY_WINDOW (activewindow));

    gtk_popover_set_pointing_to (GTK_POPOVER (fas.popover), &rect);
    gtk_widget_show (fas.popover);

    my_flow_arrow_popover_sync_controls_with_props (self);

    priv->popover_handler_instance[POPOVER_HANDLER_POPOVER_CLOSED] =
        fas.popover;
    priv->popover_handler[POPOVER_HANDLER_POPOVER_CLOSED] =
        g_signal_connect_swapped (fas.popover, "closed",
                                  G_CALLBACK (my_flow_arrow_popover_closed),
                                  self);
}

static gboolean
my_flow_arrow_button_pressed (GocItem * item, int button, double xd, double yd)
{
    MyFlowArrowPrivate *priv =
        my_flow_arrow_get_instance_private (MY_FLOW_ARROW (item));

    if (my_canvas_is_destroy_object_mode (MY_CANVAS (item->canvas))) {
        return FALSE;
    }

    MyFlowArrow *self = MY_FLOW_ARROW (item);
    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    parent_class->button_pressed (GOC_ITEM (self), button, xd, yd);

    my_flow_arrow_popover_show (MY_FLOW_ARROW (item));

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
}

void
my_flow_arrow_realize (GocItem * item)
{

    MyFlowArrow *self = MY_FLOW_ARROW (item);

    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (item);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    /* chaining up */
    parent_class->realize (item);
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
                             -G_MAXDOUBLE, G_MAXDOUBLE, -3600.0,
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

    obj_properties[PROP_UNIT] =
        g_param_spec_uint ("energy-unit",
                           "energy-unit",
                           "energy-unit",
                           0, G_MAXUINT, UNIT_WATTHOUR,
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
my_flow_arrow_update_intensity_box_of_associated_systems (MyFlowArrow * self,
                                                          GParamSpec * pspec,
                                                          gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    MyIntensityBox *ib;
    GtkWidget *widget;

    MyCanvas *canvas;
    GList *la, *ls;

    g_object_get (GOC_ITEM (self), "canvas", &canvas, NULL);

    GocGroup *group_arrows = canvas->group[GROUP_ARROWS];
    GocGroup *group_systems = canvas->group[GROUP_SYSTEMS];

    for (ls = group_systems->children; ls != NULL; ls = ls->next) {

        gdouble energy_sum = 0;

        for (la = group_arrows->children; la != NULL; la = la->next) {

            MySystem *primary, *secondary;
            gdouble energy;

            g_object_get (la->data, "primary-system", &primary,
                          "secondary-system", &secondary, "energy-quantity",
                          &energy, NULL);

            if (MY_SYSTEM (ls->data) == MY_SYSTEM (primary)) {
                energy_sum += energy;
            }
            else if (MY_SYSTEM (ls->data) == MY_SYSTEM (secondary)) {
                energy_sum -= energy;
            }
        }

        g_object_get (ls->data, "widget", &widget, NULL);

        ib = my_system_widget_get_intensity_box (MY_SYSTEM_WIDGET (widget));

        g_object_set (ib, "delta-energy", ENERGY_FACTOR * energy_sum, NULL);

        g_object_unref (widget);
    }
}

static void
my_flow_arrow_energy_quantity_changed (MyFlowArrow * self,
                                       GParamSpec * pspec, gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);
    gdouble a, b, c;

    GOStyle *style;
    GOArrow *tmp;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    g_object_get (G_OBJECT (self), "style", &style, NULL);

    /* factor 1/4 scales 250pixels to 1000 */
    style->line.width =
        ABS (ENERGY_FACTOR * priv->energy_quantity / priv->prefix_factor);

    if (priv->energy_quantity < 0) {
        tmp = priv->arrow_end;
        go_arrow_clear (priv->arrow_start);
    }
    else {
        tmp = priv->arrow_start;
        go_arrow_clear (priv->arrow_end);
    }

    a = style->line.width + 40;
    b = a;
    c = (style->line.width / 2) + (0.25 * style->line.width) + 6;

    go_arrow_init_kite (tmp, a, b, c);

    g_object_set (self, "end-arrow", priv->arrow_end, "start-arrow",
                  priv->arrow_start, NULL);

    g_object_notify (G_OBJECT (self), "label-text");
    my_flow_arrow_update (self);
}

gdouble
my_flow_arrow_get_preferred_height (MyFlowArrow * self)
{
    gdouble height_label, height_arrow_tip;

    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    if (priv->arrow_end->typ == GO_ARROW_KITE) {
        height_arrow_tip = priv->arrow_end->a;
    }
    else {
        height_arrow_tip = priv->arrow_start->a;
    }

    if (G_IS_OBJECT (priv->label))
        g_object_get (priv->label, "height", &height_label, NULL);
    else
        return height_arrow_tip + 30.0;

    return height_label + height_arrow_tip + 30.0;
}

gdouble
my_flow_arrow_get_preferred_width (MyFlowArrow * self)
{
    gdouble width_label, width_arrow_tip;

    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    if (priv->arrow_end->typ == GO_ARROW_KITE) {
        width_arrow_tip = priv->arrow_end->a;
    }
    else {
        width_arrow_tip = priv->arrow_start->a;
    }

    if (G_IS_OBJECT (priv->label))
        g_object_get (priv->label, "width", &width_label, NULL);
    else
        return width_arrow_tip + 30.0;

    return width_label + width_arrow_tip + 30.0;
}

_label_widget_clicked (MyFlowArrow * self, GdkEvent * event,
                       GtkWidget * eventbox)
{
    my_flow_arrow_popover_show(self);
}

GtkWidget *
init_label_widget (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    GtkWidget *label, *box, *label_energy;
    GtkWidget *eventbox;
    GtkStyleContext *context;
    PangoAttrList *attrs;
    gchar *str_energy;

    eventbox = gtk_event_box_new ();

    g_signal_connect_swapped (eventbox, "button-press-event",
                              G_CALLBACK (_label_widget_clicked), self);

    context = gtk_widget_get_style_context (eventbox);
    gtk_style_context_add_class (context, "label");

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    if (priv->label_text != NULL) {
        if (g_utf8_strlen (priv->label_text, -1) > 0) {
            label = gtk_label_new (NULL);

            attrs = pango_attr_list_new ();

            pango_attr_list_insert (attrs,
                                    pango_attr_weight_new
                                    (PANGO_WEIGHT_SEMIBOLD));

            pango_attr_list_insert (attrs, pango_attr_scale_new (1.4));

            gtk_label_set_attributes (GTK_LABEL (label), attrs);
            gtk_label_set_markup (GTK_LABEL (label), priv->label_text);

            gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 5);
        }
    }

    if (my_flow_arrow_is_energy_amount_visible (self)) {

        gdouble energy_quantity;
        gchar *prefix, *unit;

        energy_quantity =
            ABS (my_flow_arrow_energy_quantity_transform_to_metric_and_unit
                 (self));
        prefix = my_flow_arrow_prefix_get_name (self);
        unit = my_flow_arrow_unit_get_name (self);

        str_energy =
            g_strdup_printf ("%.3f %s%s", energy_quantity, prefix, unit);

        g_free (prefix);
        g_free (unit);

        label_energy = gtk_label_new (str_energy);

        g_free (str_energy);

        gtk_box_pack_start (GTK_BOX (box), label_energy, FALSE, FALSE, 5);
    }

    gtk_container_add (GTK_CONTAINER (eventbox), box);

    gtk_widget_show_all (eventbox);

    gtk_event_box_set_visible_window (GTK_EVENT_BOX (eventbox), TRUE);

    gtk_event_box_set_above_child (GTK_EVENT_BOX (eventbox), TRUE);

    gtk_widget_set_events (eventbox,
                           GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK |
                           GDK_BUTTON1_MOTION_MASK | GDK_BUTTON2_MOTION_MASK |
                           GDK_BUTTON3_MOTION_MASK);

    return eventbox;
}

static void
my_flow_arrow_label_text_changed (MyFlowArrow * self, GParamSpec * pspec,
                                  gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gint width, height;
    gboolean show_energy_amount;
    MyCanvas *canvas;
    GocGroup *group_labels = NULL;

    if (GOC_IS_ITEM (priv->label)) {
        goc_item_destroy (priv->label);
        priv->label = NULL;
    }

    g_object_get (self, "canvas", &canvas, NULL);

    group_labels = canvas->group[GROUP_LABELS];

    show_energy_amount = my_flow_arrow_is_energy_amount_visible (self);

    if (show_energy_amount) {
    }
    else if (!priv->label_text && !show_energy_amount) {
        my_flow_arrow_update (self);
        return;
    }
    else if (g_utf8_strlen (priv->label_text, -1) == 0) {
        my_flow_arrow_update (self);
        return;
    }

    priv->eventbox = init_label_widget (self);

    gtk_widget_get_preferred_width (priv->eventbox, NULL, &width);
    gtk_widget_get_preferred_height (priv->eventbox, NULL, &height);

    priv->label =
        goc_item_new (canvas->group[GROUP_LABELS], GOC_TYPE_WIDGET,
                      "widget", priv->eventbox, "width",
                      ((gdouble) width) + 30.0, "height",
                      ((gdouble) height), NULL);

    my_flow_arrow_update (self);
}

void
my_flow_arrow_update_while_dragged (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    cairo_rectangle_t from, dest;
    gdouble x0, y0, x1, y1;
    MyAnchorType anchor;


    g_object_get (self, "x1", &x1, "y1", &y1, NULL);

    from.width = 10;
    from.height = 10;
    from.x = x1;
    from.y = y1;

    my_system_get_allocation (priv->primary_system, &dest);

    anchor = calculate_anchor_of_dest (from, dest);

    my_system_get_coordinate_of_anchor (priv->primary_system, anchor, &x0, &y0);

    g_object_set (self, "primary-anchor", anchor, NULL);

    g_object_set (self, "x0", x0, "y0", y0, NULL);
}

void
my_flow_arrow_sync_with_associated_systems (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    cairo_rectangle_t primary_alloc, secondary_alloc;
    gdouble x0, x1, y0, y1, preferred_width, preferred_height;

    my_system_get_allocation (priv->primary_system, &primary_alloc);

    /* if arrow depicts transfer between primary and secondary system */
    if (MY_IS_SYSTEM (priv->secondary_system)) {

        my_system_get_allocation (priv->secondary_system, &secondary_alloc);

        priv->secondary_anchor =
            get_dynamic_coordinate_of_dest
            (primary_alloc, secondary_alloc, &x1, &y1);

        priv->primary_anchor =
            get_dynamic_coordinate_of_dest
            (secondary_alloc, primary_alloc, &x0, &y0);

        /*g_object_set (self, "primary-anchor", primary_anchor, */
        /*"secondary-anchor", secondary_anchor, NULL); */
    }
    /* if arrow depicts transfer between primary system and the environment */
    else {
        /* if arrow depicts transfer to environment */
        alloc_get_coordinate_of_anchor (primary_alloc,
                                        priv->primary_anchor, &x0, &y0);

        preferred_width = my_flow_arrow_get_preferred_width (self);
        preferred_height = my_flow_arrow_get_preferred_height (self);

        if (priv->primary_anchor == MY_ANCHOR_WEST) {
            x1 = x0 - preferred_width;
            y1 = y0;
        }
        else if (priv->primary_anchor == MY_ANCHOR_EAST) {
            x1 = x0 + preferred_width;
            y1 = y0;
        }
        else if (priv->primary_anchor == MY_ANCHOR_SOUTH) {
            x1 = x0;
            y1 = y0 + preferred_height;
        }
        else if (priv->primary_anchor == MY_ANCHOR_NORTH) {
            x1 = x0;
            y1 = y0 - preferred_height;
        }
    }

    g_signal_handler_block (priv->handler_instance[FLOW_ARROW_CHANGED_X1],
                            priv->handler[FLOW_ARROW_CHANGED_X1]);
    g_signal_handler_block (priv->handler_instance[FLOW_ARROW_CHANGED_Y1],
                            priv->handler[FLOW_ARROW_CHANGED_Y1]);

    my_flow_arrow_set_coordinate (self, "x0", x0,
                                  "y0", y0, "x1", x1, "y1", y1, NULL);

    g_object_set (priv->drag_point, "x", x1, "y", y1, NULL);

    g_signal_handler_unblock (priv->handler_instance[FLOW_ARROW_CHANGED_X1],
                              priv->handler[FLOW_ARROW_CHANGED_X1]);
    g_signal_handler_unblock (priv->handler_instance[FLOW_ARROW_CHANGED_Y1],
                              priv->handler[FLOW_ARROW_CHANGED_Y1]);
}

void
my_flow_arrow_update (MyFlowArrow * self)
{

    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    if (priv->is_dragged) {
        my_flow_arrow_update_while_dragged (self);
    }
    else {
        my_flow_arrow_sync_with_associated_systems (self);
    }

    my_flow_arrow_update_label (self, NULL, NULL);

    g_object_notify (G_OBJECT (self), "transfer-type");
}

void
_secondary_system_finalized (gpointer data, GObject * old)
{
    MyFlowArrow *self = data;

    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    priv->secondary_system = NULL;

    g_object_set (self, "secondary-system", NULL, NULL);

    my_flow_arrow_update (self);
}

void
_primary_system_finalized (gpointer data, GObject * old)
{
    MyFlowArrow *self = data;

    goc_item_destroy (GOC_ITEM (self));
}

static void
my_flow_arrow_sync_coordinate_with_drag_point_coordinate (MyFlowArrow * self,
                                                          GParamSpec * pspec,
                                                          gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);
    gdouble x1, y1;

    guint handler[] = { DRAG_POINT_NW_CHANGED_X, DRAG_POINT_NW_CHANGED_Y };
    guint n;

    g_return_if_fail (MY_IS_SYSTEM (priv->primary_system));
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    g_object_get (self, "x1", &x1, "y1", &y1, NULL);

    for (n = 0; n < sizeof (handler) / sizeof (handler[0]); n++) {
        if (priv->handler[handler[n]] != 0)
            g_signal_handler_block (priv->handler_instance[handler[n]],
                                    priv->handler[handler[n]]);
    }

    g_object_set (priv->drag_point, "x", x1, "y", y1, NULL);

    my_flow_arrow_update (self);

    for (n = 0; n < sizeof (handler) / sizeof (handler[0]); n++) {
        if (priv->handler[handler[n]] != 0)
            g_signal_handler_unblock (priv->handler_instance[handler[n]],
                                      priv->handler[handler[n]]);
    }
}

static void
my_flow_arrow_sync_drag_point_coordinate_with_self_coordinate (MyFlowArrow *
                                                               self,
                                                               GParamSpec *
                                                               pspec,
                                                               MyDragPoint *
                                                               point)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gdouble x_dp, y_dp;

    guint handler[] = { FLOW_ARROW_CHANGED_Y1, DRAG_POINT_NW_CHANGED_X,
        DRAG_POINT_NW_CHANGED_Y, FLOW_ARROW_CHANGED_X1
    };
    guint n;

    g_object_get (point, "x", &x_dp, "y", &y_dp, NULL);

    for (n = 0; n < sizeof (handler) / sizeof (handler[0]); n++)
        g_signal_handler_block (priv->handler_instance[handler[n]],
                                priv->handler[handler[n]]);

    g_object_set (self, "x1", x_dp, "y1", y_dp, NULL);

    my_flow_arrow_update (self);

    for (n = 0; n < sizeof (handler) / sizeof (handler[0]); n++)
        g_signal_handler_unblock (priv->handler_instance[handler[n]],
                                  priv->handler[handler[n]]);
}

static gboolean
my_flow_arrow_is_energy_amount_visible (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    GtkWindow *activewindow;
    GSimpleAction *show_energy_amount;
    GVariant *state;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    activewindow = my_application_get_active_window ();

    show_energy_amount = (GSimpleAction *)
        g_action_map_lookup_action (G_ACTION_MAP (activewindow),
                                    "show-energy-amount-of-flow-arrows");

    g_object_get (show_energy_amount, "state", &state, NULL);

    return g_variant_get_boolean (state);
}

static void
my_flow_arrow_show_energy_amount_state_changed (MyFlowArrow * self,
                                                GParamSpec * pspec,
                                                GAction * action)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_object_notify (G_OBJECT (self), "label-text");
}

static void
my_flow_arrow_app_window_changed_metric_prefix (MyFlowArrow * self,
                                                GParamSpec * pspec,
                                                MyWindow * window)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gdouble factor;

    factor = my_window_get_metric_prefix_factor (window);

    priv->energy_quantity *= factor / priv->prefix_factor;

    priv->prefix_factor = factor;

    g_object_notify (G_OBJECT (self), "energy-quantity");
    g_object_notify (G_OBJECT (self), "label-text");
}

static void
my_flow_arrow_canvas_changed (MyFlowArrow * self,
                              GParamSpec * pspec, gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    MyCanvas *canvas;
    GocGroup *group_dragpoints;
    GtkWindow *activewindow;
    GSimpleAction *show_energy_amount;
    guint i;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    for (i = 0; i < N_HANDLER; i++) {
        if (priv->handler[i] != 0 && G_IS_OBJECT (priv->handler_instance[i])) {
            if (g_signal_handler_is_connected
                (priv->handler_instance[i], priv->handler[i])) {
                g_signal_handler_disconnect (priv->handler_instance[i],
                                             priv->handler[i]);
            }
        }
    }

    g_object_get (self, "canvas", &canvas, NULL);

    if (!MY_IS_CANVAS (canvas)) {
        return;
    }

    g_object_unref (canvas);

    activewindow = my_application_get_active_window ();

    show_energy_amount = (GSimpleAction *)
        g_action_map_lookup_action (G_ACTION_MAP (activewindow),
                                    "show-energy-amount-of-flow-arrows");

    group_dragpoints = canvas->group[GROUP_ARROW_DRAGPOINTS];

    g_return_if_fail (GOC_IS_GROUP (group_dragpoints));

    if (!MY_IS_DRAG_POINT (priv->drag_point)) {
        priv->drag_point = (MyDragPoint *)
            goc_item_new (group_dragpoints, MY_TYPE_DRAG_POINT,
                          "linked-item", self, NULL);
    }

    priv->handler_instance[CANVAS_CHANGED_ENERGY_QUANTITY_UPDATE_INTENSITY_BOX]
        = self;
    priv->handler[CANVAS_CHANGED_ENERGY_QUANTITY_UPDATE_INTENSITY_BOX] =
        g_signal_connect (self, "notify::energy-quantity",
                          G_CALLBACK
                          (my_flow_arrow_update_intensity_box_of_associated_systems),
                          NULL);

    priv->handler_instance[CANVAS_CHANGED_PRIMARY_SYSTEM_UPDATE_INTENSITY_BOX] =
        self;
    priv->handler[CANVAS_CHANGED_PRIMARY_SYSTEM_UPDATE_INTENSITY_BOX] =
        g_signal_connect (self, "notify::primary-system",
                          G_CALLBACK
                          (my_flow_arrow_update_intensity_box_of_associated_systems),
                          NULL);

    priv->handler_instance[CANVAS_CHANGED_SECONDARY_SYSTEM_UPDATE_INTENSITY_BOX]
        = self;
    priv->handler[CANVAS_CHANGED_SECONDARY_SYSTEM_UPDATE_INTENSITY_BOX] =
        g_signal_connect (self, "notify::secondary-system",
                          G_CALLBACK
                          (my_flow_arrow_update_intensity_box_of_associated_systems),
                          NULL);

    priv->handler_instance[CANVAS_CHANGED_ENERGY_QUANTITY] = self;
    priv->handler[CANVAS_CHANGED_ENERGY_QUANTITY] =
        g_signal_connect (self, "notify::energy-quantity",
                          G_CALLBACK (my_flow_arrow_energy_quantity_changed),
                          NULL);

    priv->handler_instance[CANVAS_CHANGED_LABEL_TEXT] = self;
    priv->handler[CANVAS_CHANGED_LABEL_TEXT] =
        g_signal_connect (self, "notify::label-text",
                          G_CALLBACK (my_flow_arrow_label_text_changed), NULL);

    priv->handler_instance[CANVAS_CHANGED_TRANSFER_TYPE] = self;
    priv->handler[CANVAS_CHANGED_TRANSFER_TYPE] =
        g_signal_connect (self, "notify::transfer-type",
                          G_CALLBACK (my_flow_arrow_transfer_type_changed),
                          NULL);

    priv->handler_instance[FLOW_ARROW_CHANGED_X1] = self;
    priv->handler[FLOW_ARROW_CHANGED_X1] =
        g_signal_connect (self, "notify::x1",
                          G_CALLBACK
                          (my_flow_arrow_sync_coordinate_with_drag_point_coordinate),
                          NULL);

    priv->handler_instance[FLOW_ARROW_CHANGED_Y1] = self;
    priv->handler[FLOW_ARROW_CHANGED_Y1] =
        g_signal_connect (self, "notify::y1",
                          G_CALLBACK
                          (my_flow_arrow_sync_coordinate_with_drag_point_coordinate),
                          NULL);

    priv->handler_instance[DRAG_POINT_NW_CHANGED_X] = priv->drag_point;
    priv->handler[DRAG_POINT_NW_CHANGED_X] =
        g_signal_connect_swapped (priv->drag_point, "notify::x",
                                  G_CALLBACK
                                  (my_flow_arrow_sync_drag_point_coordinate_with_self_coordinate),
                                  self);

    priv->handler_instance[DRAG_POINT_NW_CHANGED_Y] = priv->drag_point;
    priv->handler[DRAG_POINT_NW_CHANGED_Y] =
        g_signal_connect_swapped (priv->drag_point, "notify::y",
                                  G_CALLBACK
                                  (my_flow_arrow_sync_drag_point_coordinate_with_self_coordinate),
                                  self);

    priv->handler_instance[APP_WINDOW_CHANGED_METRIC_PREFIX] = activewindow;
    priv->handler[APP_WINDOW_CHANGED_METRIC_PREFIX] =
        g_signal_connect_swapped (activewindow, "notify::metric-prefix",
                                  G_CALLBACK
                                  (my_flow_arrow_app_window_changed_metric_prefix),
                                  self);

    priv->handler_instance[ACTION_SHOW_ENERGY_AMOUNT_STATE_CHANGED] =
        show_energy_amount;
    priv->handler[ACTION_SHOW_ENERGY_AMOUNT_STATE_CHANGED] =
        g_signal_connect_swapped (show_energy_amount, "change-state",
                                  G_CALLBACK
                                  (my_flow_arrow_show_energy_amount_state_changed),
                                  self);

    g_object_notify (G_OBJECT (self), "energy-quantity");
    g_object_notify (G_OBJECT (self), "label-text");
    g_object_notify (G_OBJECT (self), "transfer-type");
}


static void
my_flow_arrow_update_label (MyFlowArrow * self,
                            GParamSpec * pspec, gpointer data)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    gdouble x0, x1, y0, y1, width, height, dx, dy;
    gdouble angle;
    cairo_matrix_t matrix;

    g_object_get (self, "x0", &x0, "y0", &y0, "x1", &x1, "y1", &y1, NULL);

    if (!GOC_IS_WIDGET (priv->label))
        return;

    g_object_get (priv->label, "width", &width, "height", &height, NULL);

    angle = atan2 (y1 - y0, x1 - x0);

    if (angle < 0) {
        angle += 2 * M_PI;
    }

    if (priv->arrow_start->typ == GO_ARROW_NONE) {
        dx = priv->arrow_end->a;
    }
    else {
        dx = -priv->arrow_start->a;
    }

    gdouble l_arrow = sqrt (pow (x1 - x0, 2.0) + pow (y1 - y0, 2.0));
    gdouble lm = (l_arrow - dx) / 2.0;
    gdouble ym = sin (angle) * lm;
    gdouble xm = cos (angle) * lm;

    goc_item_set (priv->label, "x", x0 + xm - width / 2.0, "y",
                  y0 + ym - height / 2.0, NULL);

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
    priv->arrow_start = g_new0 (GOArrow, 1);
    priv->arrow_end = g_new0 (GOArrow, 1);
    priv->provider = gtk_css_provider_new ();
    priv->prefix_factor = 1.0;
    priv->transfer_type = MY_TRANSFER_WORK;
    priv->unit = UNIT_JOULE;

    style = go_style_dup (go_styled_object_get_style (GO_STYLED_OBJECT (self)));
    style->line.cap = CAIRO_LINE_CAP_ROUND;
    go_styled_object_set_style (GO_STYLED_OBJECT (self), style);

    g_object_unref (style);

    for (i = 0; i < N_HANDLER; i++) {
        priv->handler[i] = 0;
    }

    g_signal_connect (self, "notify::canvas",
                      G_CALLBACK (my_flow_arrow_canvas_changed), NULL);
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

    if (MY_IS_SYSTEM (priv->primary_system))
        g_object_weak_unref (G_OBJECT (priv->primary_system),
                             _primary_system_finalized, self);

    if (MY_IS_SYSTEM (priv->secondary_system))
        g_object_weak_unref (G_OBJECT (priv->secondary_system),
                             _secondary_system_finalized, self);

    g_free (priv->arrow_start);
    g_free (priv->arrow_end);

    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_flow_arrow_parent_class)->finalize (object);
}

/* public methods */

void
my_flow_arrow_begin_drag (MyFlowArrow * self)
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
my_flow_arrow_end_drag (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    priv->is_dragged = FALSE;

    g_object_notify (G_OBJECT (priv->primary_system), "x");
}

MyDragPoint *
my_flow_arrow_get_drag_point (MyFlowArrow * self)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    return priv->drag_point;
}

void
my_flow_arrow_drag_points_set_visible (MyFlowArrow * self, gboolean visible)
{
    MyFlowArrowPrivate *priv = my_flow_arrow_get_instance_private (self);

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    if (MY_IS_DRAG_POINT (priv->drag_point)) {
        goc_item_set_visible (GOC_ITEM (priv->drag_point), visible);
    }
}

void
my_flow_arrow_drag_points_show (MyFlowArrow * self)
{
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    my_flow_arrow_drag_points_set_visible (self, TRUE);
}

void
my_flow_arrow_drag_points_hide (MyFlowArrow * self)
{
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    my_flow_arrow_drag_points_set_visible (self, FALSE);
}
