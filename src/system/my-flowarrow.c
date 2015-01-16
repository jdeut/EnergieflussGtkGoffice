#include "my-flowarrow.h"

enum
{
    PROP_0,
    /* property entries */
    PROP_LABEL_TEXT,
    PROP_ENERGY_QUANTITY,
    PROP_LINKED_SYSTEM,
    PROP_SECONDARY_SYSTEM,
    PROP_X_I,
    PROP_Y_I,
    PROP_ANCHOR,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* 'private'/'protected' functions */
static void my_flow_arrow_class_init (MyFlowArrowClass * klass);
static void my_flow_arrow_init (MyFlowArrow * self);
static void my_flow_arrow_finalize (GObject *);
static void my_flow_arrow_dispose (GObject *);

struct _MyFlowArrowPrivate
{
    gchar *label_text;
    gboolean is_dragged;
    GOArrow *arrow;
    GocItem *label;
    MyAnchorType anchor;
    MyDragPoint *drag_point;
    gdouble x_i, y_i;
    GBinding *bind_drag_point_of_self_x;
    GBinding *bind_drag_point_of_self_y;
    MySystem *linked_system;
    MySystem *secondary_system;
    gdouble energy_quantity;
    /* private members go here */
};

#define MY_TYPE_ANCHOR_TYPE my_anchor_type_get_type ()
GType my_anchor_type_get_type (void);

GType
my_anchor_type_get_type (void)
{
    static GType etype = 0;

    if (etype == 0) {
        static GEnumValue const values[] = {
            {MY_ANCHOR_CENTER, "MY_ANCHOR_CENTER", "center"},
            {MY_ANCHOR_NORTH_WEST, "MY_ANCHOR_NORTH_WEST", "north-west"},
            {MY_ANCHOR_NORTH_EAST, "MY_ANCHOR_NORTH_EAST", "north-east"},
            {MY_ANCHOR_SOUTH, "MY_ANCHOR_SOUTH", "south"},
            {MY_ANCHOR_SOUTH_WEST, "MY_ANCHOR_SOUTH_WEST", "south-west"},
            {MY_ANCHOR_SOUTH_EAST, "MY_ANCHOR_SOUTH_EAST", "south-east"},
            {MY_ANCHOR_WEST, "MY_ANCHOR_WEST", "west"},
            {MY_ANCHOR_EAST, "MY_ANCHOR_EAST", "east"},
            {MY_ANCHOR_BASELINE_CENTER, "MY_ANCHOR_BASELINE_CENTER",
             "baseline-centered"},
            {MY_ANCHOR_BASELINE_WEST, "MY_ANCHOR_BASELINE_WEST",
             "baseline-west"},
            {MY_ANCHOR_BASELINE_EAST, "MY_ANCHOR_BASELINE_EAST",
             "baseline-east"},
            {MY_ANCHOR_N, "MY_ANCHOR_N", "n"},
            {MY_ANCHOR_NW, "MY_ANCHOR_NW", "nw"},
            {MY_ANCHOR_NE, "MY_ANCHOR_NE", "ne"},
            {MY_ANCHOR_S, "MY_ANCHOR_S", "s"},
            {MY_ANCHOR_SW, "MY_ANCHOR_SW", "sw"},
            {MY_ANCHOR_SE, "MY_ANCHOR_SE", "se"},
            {MY_ANCHOR_W, "MY_ANCHOR_W", "w"},
            {MY_ANCHOR_E, "MY_ANCHOR_E", "e"},
            {MY_ANCHOR_B, "MY_ANCHOR_B", "b"},
            {MY_ANCHOR_BW, "MY_ANCHOR_BW", "bw"},
            {MY_ANCHOR_BE, "MY_ANCHOR_BE", "be"},
            {0, NULL, NULL}
        };
        etype =
            g_enum_register_static (g_intern_static_string ("MyAnchorType"),
                                    values);
    }
    return etype;
}

#define MY_FLOW_ARROW_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                       MY_TYPE_FLOW_ARROW, \
                                       MyFlowArrowPrivate))

G_DEFINE_TYPE (MyFlowArrow, my_flow_arrow, GOC_TYPE_LINE);

static void
my_flow_arrow_set_property (GObject * object,
                            guint property_id,
                            const GValue * value, GParamSpec * pspec)
{
    MyFlowArrow *self = MY_FLOW_ARROW (object);

    switch (property_id) {

        case PROP_LABEL_TEXT:
            g_free (self->_priv->label_text);
            self->_priv->label_text = g_value_dup_string (value);
            break;

        case PROP_LINKED_SYSTEM:{
                MySystem *linked_system =
                    MY_SYSTEM (g_value_get_object (value));
                my_flow_arrow_set_linked_system (self, linked_system);
                return;
            }
        case PROP_SECONDARY_SYSTEM:
            self->_priv->secondary_system = g_value_get_object (value);
            break;
        case PROP_ENERGY_QUANTITY:
            self->_priv->energy_quantity = g_value_get_double (value);
            break;
        case PROP_ANCHOR:
            self->_priv->anchor = (MyAnchorType) g_value_get_enum (value);
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

    switch (property_id) {

        case PROP_LABEL_TEXT:
            g_value_set_string (value, self->_priv->label_text);
            break;

        case PROP_LINKED_SYSTEM:
            g_value_set_object (value, self->_priv->linked_system);
            break;

        case PROP_SECONDARY_SYSTEM:
            g_value_set_object (value, self->_priv->secondary_system);
            break;

        case PROP_X_I:
            g_value_set_double (value, self->_priv->x_i);
            break;

        case PROP_Y_I:
            g_value_set_double (value, self->_priv->y_i);
            break;

        case PROP_ENERGY_QUANTITY:
            g_value_set_double (value, self->_priv->energy_quantity);
            break;

        case PROP_ANCHOR:
            g_value_set_enum (value, self->_priv->anchor);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
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
my_flow_arrow_draw_label (MyFlowArrow * self, cairo_t * cr)
{

    GocGroup *toplevel = NULL;
    gdouble x0, x1, y0, y1;

    toplevel = goc_canvas_get_root (GOC_ITEM (self)->canvas);

    g_object_get (self, "x0", &x0, "x1", &x1, "y0", &y0, "y1", &y1, NULL);

    if (self->_priv->label_text != NULL) {

        if (!GOC_IS_TEXT (self->_priv->label)) {
            GError *err = NULL;
            gchar *text;

            PangoAttrList *attr;

            attr = pango_attr_list_new ();

            pango_parse_markup (self->_priv->label_text, -1, 0, &attr,
                                &text, NULL, &err);

            if (err != NULL) {
                g_printerr ("Error parsing str '%s': %s\n",
                            self->_priv->label_text, err->message);
                g_clear_error (&err);

            }
            else {

                self->_priv->label =
                    goc_item_new (toplevel, GOC_TYPE_TEXT, "attributes", attr,
                                  "text", text, NULL);

                goc_item_lower_to_bottom (GOC_ITEM (self->_priv->label));
            }
        }


        if (GOC_IS_TEXT (self->_priv->label)) {
            gdouble angle;
            cairo_matrix_t matrix;

            g_object_get (self, "x0", &x0, "x1", &x1, "y0", &y0, "y1", &y1,
                          NULL);

            angle = atan2 (y1 - y0, x1 - x0);

            if (angle < 0) {
                angle += 2 * M_PI;
            }

            goc_item_set (self->_priv->label, "rotation", angle, "anchor",
                          GO_ANCHOR_SOUTH, "x", x0 + (x1 - x0) / 2, "y",
                          y0 + (y1 - y0) / 2, NULL);

            cairo_matrix_init_identity (&matrix);
            cairo_matrix_translate (&matrix,
                                    self->_priv->energy_quantity / 2 *
                                    sin (angle),
                                    -self->_priv->energy_quantity / 2 *
                                    cos (angle));

            goc_item_set_transform (self->_priv->label, &matrix);
        }
    }
}

void
my_flow_arrow_draw (GocItem const *item, cairo_t * cr)
{
    MyFlowArrow *self = MY_FLOW_ARROW (item);

    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    /* chaining up */
    parent_class->draw (GOC_ITEM (self), cr);

    my_flow_arrow_draw_label (self, cr);

    /*if (GOC_IS_ITEM (self->_priv->drag_point)) { */
    /*g_object_set (self->_priv->drag_point, "x", x1, "y", y1, NULL); */
    /*} */

    gtk_widget_queue_draw (GTK_WIDGET (item->canvas));
}

static void
my_flow_arrow_init_style (G_GNUC_UNUSED GocStyledItem * item, GOStyle * style)
{
    style->interesting_fields = GO_STYLE_LINE;
    if (style->line.auto_dash)
        style->line.dash_type = GO_LINE_SOLID;
    if (style->line.auto_color)
        style->line.color = GO_COLOR_FROM_RGBA (0, 200, 0, 255);
    if (style->line.auto_fore)
        style->line.fore = 0;
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
                             "Label text",
                             NULL, G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    obj_properties[PROP_ENERGY_QUANTITY] =
        g_param_spec_double ("energy-quantity",
                             "energy-quantity",
                             "The energy quantity that is transfered",
                             -G_MAXDOUBLE, G_MAXDOUBLE, 1,
                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    obj_properties[PROP_X_I] =
        g_param_spec_double ("xi",
                             "intermediate x coordinate",
                             "...",
                             -G_MAXDOUBLE, G_MAXDOUBLE, 1,
                             G_PARAM_READABLE);

    obj_properties[PROP_Y_I] =
        g_param_spec_double ("yi",
                             "intermediate y coordinate",
                             "...",
                             -G_MAXDOUBLE, G_MAXDOUBLE, 1,
                             G_PARAM_READABLE);

    obj_properties[PROP_LINKED_SYSTEM] =
        g_param_spec_object ("linked-system",
                             "linked system",
                             "A pointer to the linked system from where the energy transport is seen",
                             MY_TYPE_SYSTEM, G_PARAM_READWRITE);

    obj_properties[PROP_SECONDARY_SYSTEM] =
        g_param_spec_object ("secondary-system",
                             "secondary system",
                             "A pointer to the secondary system where the arrow is connected to",
                             MY_TYPE_SYSTEM, G_PARAM_READWRITE);

    obj_properties[PROP_ANCHOR] =
        g_param_spec_enum ("anchor",
                           "anchor",
                           "Where the arrow should snap at the linked-system",
                           MY_TYPE_ANCHOR_TYPE, 0, G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);

    g_type_class_add_private (gobject_class, sizeof (MyFlowArrowPrivate));

    gi_class->draw = my_flow_arrow_draw;
    gi_class->enter_notify = my_flow_arrow_enter_notify;
    gi_class->leave_notify = my_flow_arrow_leave_notify;
    gi_class->button_pressed = my_flow_arrow_button_pressed;

    gsi_class->init_style = my_flow_arrow_init_style;
}

void
bind_s_to_t1_if_bound_to_t2 (GBinding ** binding,
                             GObject * source,
                             GObject * target,
                             const gchar * s,
                             const gchar * t1, const gchar * t2)
{

    if (!G_IS_BINDING (*binding)) {
        *binding =
            g_object_bind_property (source, s, target, t1,
                                    G_BINDING_BIDIRECTIONAL);
    }
    else if (strcmp (g_binding_get_target_property (*binding), t2) == 0) {

        g_object_unref (*binding);

        *binding =
            g_object_bind_property (source, s, target, t1,
                                    G_BINDING_BIDIRECTIONAL);
    }
}

static void
my_flow_arrow_energy_quantity_changed (MyFlowArrow * self, GParamSpec * pspec,
                                       gpointer data)
{
    GOStyle *style;

    g_object_get (G_OBJECT (self), "style", &style, NULL);

    style->line.width = ABS (self->_priv->energy_quantity);

    if(!MY_IS_DRAG_POINT(self->_priv->drag_point)) {
        return;
    }

    /* if energy_quantity < 0.0 than arrow tip ends on calculated anchor point on the secondary_system */
    if (self->_priv->energy_quantity < 0.0) {

        bind_s_to_t1_if_bound_to_t2 (&self->_priv->bind_drag_point_of_self_x,
                                     G_OBJECT (self->_priv->drag_point),
                                     G_OBJECT (self), "x", "x1", "x0");

        bind_s_to_t1_if_bound_to_t2 (&self->_priv->bind_drag_point_of_self_y,
                                     G_OBJECT (self->_priv->drag_point),
                                     G_OBJECT (self), "y", "y1", "y0");
    }
    /* if energy_quantity > 0 than arrow tip ends on calculated anchor point on the primary_system,
     * so that the tail of the arrow is in the center of the secondary_system */
    else {

        bind_s_to_t1_if_bound_to_t2 (&self->_priv->bind_drag_point_of_self_x,
                                     G_OBJECT (self->_priv->drag_point),
                                     G_OBJECT (self), "x", "x0", "x1");

        bind_s_to_t1_if_bound_to_t2 (&self->_priv->bind_drag_point_of_self_y,
                                     G_OBJECT (self->_priv->drag_point),
                                     G_OBJECT (self), "y", "y0", "y1");
    }
}

static void
notify_label_text_changed_cb (MyFlowArrow * self, GParamSpec * pspec,
                              gpointer data)
{
    GOStyle *style;
    GtkWidget *canvas;

    canvas = GTK_WIDGET (GOC_ITEM (self)->canvas);

    if (G_IS_OBJECT (self->_priv->label)) {
        g_object_unref (self->_priv->label);
    }

    if (GTK_IS_WIDGET (canvas)) {
        gtk_widget_queue_draw (canvas);
    }
}

static void
my_flow_arrow_canvas_changed (MyFlowArrow * self, GParamSpec * pspec,
                              gpointer data)
{
    MyCanvas *canvas;
    GocGroup *group_arrows;

    g_object_get (self, "canvas", &canvas, NULL);

    group_arrows = canvas->group_arrows;

    self->_priv->drag_point = (MyDragPoint *)
        goc_item_new (group_arrows, MY_TYPE_DRAG_POINT, "radius", 5.0,
                      "linked-item", self, NULL);

    g_signal_connect (self, "notify::energy-quantity",
                      G_CALLBACK (my_flow_arrow_energy_quantity_changed), NULL);

    my_flow_arrow_energy_quantity_changed (self, NULL, NULL);

    g_signal_connect (self, "notify::label-text",
                      G_CALLBACK (notify_label_text_changed_cb), NULL);
}

static void
my_flow_arrow_init (MyFlowArrow * self)
{
    GOStyle *style;

    /*group_arrows = canvas->group_arrows; */

    self->_priv = MY_FLOW_ARROW_GET_PRIVATE (self);

    /* to init any of the private data, do e.g: */

    self->_priv->is_dragged = FALSE;
    self->_priv->arrow = g_new0 (GOArrow, 1);

    go_arrow_init_kite (self->_priv->arrow, 20, 20, 4);

    g_object_set (self, "end-arrow", self->_priv->arrow, NULL);

    g_signal_connect (self, "notify::canvas",
                      G_CALLBACK (my_flow_arrow_canvas_changed), NULL);
}

void
my_flow_arrow_destroy (MyFlowArrow * self)
{

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    if (GOC_IS_TEXT (self->_priv->label)) {
        goc_item_destroy (GOC_ITEM (self->_priv->label));
    }

    if (MY_IS_DRAG_POINT (self->_priv->drag_point)) {
        goc_item_destroy (GOC_ITEM (self->_priv->drag_point));
    }

    g_free (self->_priv->arrow);

    g_object_run_dispose (G_OBJECT (self));
    g_object_unref (self);
}

static void
my_flow_arrow_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_flow_arrow_parent_class)->dispose (object);
}

static void
my_flow_arrow_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_flow_arrow_parent_class)->finalize (object);
}

/* public methods */

void
my_flow_arrow_begin_dragging (MyFlowArrow * self)
{
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    self->_priv->is_dragged = TRUE;
}

gboolean
my_flow_arrow_is_dragged (MyFlowArrow * self)
{
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    return self->_priv->is_dragged;
}

void
my_flow_arrow_end_dragging (MyFlowArrow * self)
{
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    self->_priv->is_dragged = FALSE;
}

void
my_flow_arrow_show_drag_points (MyFlowArrow * self)
{

    MyCanvas *canvas;
    GocGroup *group_arrows = NULL;
    gdouble x1, y1;

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    goc_item_show (GOC_ITEM (self->_priv->drag_point));
}

void
my_flow_arrow_hide_drag_points (MyFlowArrow * self)
{
    if (MY_IS_DRAG_POINT (self->_priv->drag_point)) {
        goc_item_hide (GOC_ITEM (self->_priv->drag_point));
    }
}

MySystem *
my_flow_arrow_get_linked_system (MyFlowArrow * self)
{

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    return self->_priv->linked_system;
}

void
my_flow_arrow_set_linked_system (MyFlowArrow * self, MySystem * system)
{

    g_return_if_fail (MY_IS_SYSTEM (system));
    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    self->_priv->linked_system = system;
}
