#include "my-flowarrow.h"

enum
{
    PROP_0,
    /* property entries */
    PROP_LABEL_TEXT,
    PROP_ENERGY_QUANTITY,
    PROP_LINKED_SYSTEM,
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
    GOArrow *arrow;
    GocItem *label;
    MyDragPoint *drag_point;
    MySystem *linked_system;
    gfloat energy_quantity;
    /* private members go here */
};

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
        case PROP_ENERGY_QUANTITY:
            self->_priv->energy_quantity = g_value_get_double (value);
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

        case PROP_ENERGY_QUANTITY:
            g_value_set_double (value, self->_priv->energy_quantity);
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
my_flow_arrow_draw (GocItem const *item, cairo_t * cr)
{
    GocGroup *toplevel = NULL;
    gdouble x0, x1, y0, y1;
    MyFlowArrow *self = MY_FLOW_ARROW (item);

    MyFlowArrowClass *class = MY_FLOW_ARROW_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    toplevel = goc_canvas_get_root (item->canvas);

    /* chaining up */
    parent_class->draw (GOC_ITEM (self), cr);

    g_object_get (self, "x0", &x0, "x1", &x1, "y0", &y0, "y1", &y1,
                  NULL);

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

    if (GOC_IS_ITEM (self->_priv->drag_point)) {
        g_object_set (self->_priv->drag_point, "x", x1, "y", y1, NULL);
    }

    gtk_widget_queue_draw (GTK_WIDGET (item->canvas));
}

static void
my_flow_arrow_class_init (MyFlowArrowClass * klass)
{
    GObjectClass *gobject_class;
    GocItemClass *gi_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gi_class = (GocItemClass *) klass;

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
                             0, G_MAXDOUBLE, 1,
                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    obj_properties[PROP_LINKED_SYSTEM] =
        g_param_spec_object ("linked-system",
                             "linked system",
                             "A pointer to the linked system",
                             MY_TYPE_SYSTEM, G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);

    g_type_class_add_private (gobject_class, sizeof (MyFlowArrowPrivate));

    gi_class->draw = my_flow_arrow_draw;
    gi_class->enter_notify = my_flow_arrow_enter_notify;
    gi_class->leave_notify = my_flow_arrow_leave_notify;
    gi_class->button_pressed = my_flow_arrow_button_pressed;
}

static void
notify_canvas_changed_cb (MyFlowArrow * self, GParamSpec * pspec, gpointer data)
{
    GOStyle *style;

    goc_item_lower_to_bottom (GOC_ITEM (self));
    g_object_get (G_OBJECT (self), "style", &style, NULL);

    style->line.width = self->_priv->energy_quantity;
    style->line.color = GO_COLOR_FROM_RGBA (0, 200, 0, 255);
}

static void
notify_energy_quantity_changed_cb (MyFlowArrow * self, GParamSpec * pspec,
                                   gpointer data)
{
    GOStyle *style;
    GtkWidget *canvas;

    g_object_get (G_OBJECT (self), "style", &style, NULL);

    style->line.width = self->_priv->energy_quantity;
    canvas = GTK_WIDGET (GOC_ITEM (self)->canvas);

    if (GTK_IS_WIDGET (canvas))
        gtk_widget_queue_draw (canvas);
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
my_flow_arrow_init (MyFlowArrow * self)
{
    self->_priv = MY_FLOW_ARROW_GET_PRIVATE (self);

    /* to init any of the private data, do e.g: */

    self->_priv->arrow = g_new0 (GOArrow, 1);

    go_arrow_init_kite (self->_priv->arrow, 20, 20, 4);

    g_object_set (self, "end-arrow", self->_priv->arrow, NULL);

    g_signal_connect (self, "notify::canvas",
                      G_CALLBACK (notify_canvas_changed_cb), NULL);

    g_signal_connect (self, "notify::energy-quantity",
                      G_CALLBACK (notify_energy_quantity_changed_cb), NULL);

    g_signal_connect (self, "notify::label-text",
                      G_CALLBACK (notify_label_text_changed_cb), NULL);
}

void
my_flow_arrow_destroy (MyFlowArrow * self)
{

    g_return_if_fail (MY_IS_FLOW_ARROW (self));

    g_signal_handlers_disconnect_by_func (self, notify_canvas_changed_cb, NULL);

    if (GOC_IS_TEXT (self->_priv->label)) {
        goc_item_destroy (GOC_ITEM (self->_priv->label));
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
my_flow_arrow_show_drag_points (MyFlowArrow * self)
{

    GocGroup *toplevel = NULL;
    gdouble x1, y1;

    g_object_get (self, "x1", &x1, "y1", &y1, NULL);

    toplevel = goc_canvas_get_root (GOC_ITEM(self)->canvas);

    if (!MY_IS_DRAG_POINT (self->_priv->drag_point)) {
        self->_priv->drag_point = (MyDragPoint*)
            goc_item_new (toplevel, MY_TYPE_DRAG_POINT, "x", x1, "y", y1, "radius",
                          5.0, NULL);
    }
    else {
        g_object_set (self->_priv->drag_point, "x", x1, "y", y1, NULL);
        goc_item_show (GOC_ITEM(self->_priv->drag_point));
    }
}

void
my_flow_arrow_hide_drag_points (MyFlowArrow * self)
{
    if (MY_IS_DRAG_POINT (self->_priv->drag_point)) {
        goc_item_hide (GOC_ITEM(self->_priv->drag_point));
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
