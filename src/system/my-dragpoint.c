#include "my-dragpoint.h"

enum
{
    PROP_0,
    /* property entries */
    PROP_LINKED_ITEM,
    N_PROPERTIES
};


static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* 'private'/'protected' functions */
static void my_drag_point_class_init (MyDragPointClass * klass);
static void my_drag_point_init (MyDragPoint * self);
static void my_drag_point_finalize (GObject * );
static void my_drag_point_dispose (GObject * );

struct _MyDragPointPrivate
{
    GocItem *linked_item;
    gboolean is_dragged;
};

G_DEFINE_TYPE_WITH_PRIVATE (MyDragPoint, my_drag_point, GOC_TYPE_CIRCLE);

GQuark
my_drag_point_error_quark (void)
{
  return g_quark_from_static_string ("my-drag-point-error-quark");
}

static void
my_drag_point_set_property (GObject * object,
                            guint property_id,
                            const GValue * value, GParamSpec * pspec)
{
    MyDragPoint *self = MY_DRAG_POINT (object);
    MyDragPointPrivate *priv = my_drag_point_get_instance_private(self);

    switch (property_id) {

        case PROP_LINKED_ITEM: 
                priv->linked_item = (GocItem*) g_value_get_object (value);
                return;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_drag_point_get_property (GObject * object,
                            guint property_id,
                            GValue * value, GParamSpec * pspec)
{
    MyDragPoint *self = MY_DRAG_POINT (object);
    MyDragPointPrivate *priv = my_drag_point_get_instance_private(self);

    switch (property_id) {

        case PROP_LINKED_ITEM:
            g_value_set_object (value, priv->linked_item);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

gboolean
my_drag_point_button_pressed (GocItem * item, int button, double x, double y)
{
    MyDragPointPrivate *priv = my_drag_point_get_instance_private(MY_DRAG_POINT(item));

    GocItemClass *gi_class = my_drag_point_parent_class;

    gi_class->button_pressed (item, button, x, y);

    return FALSE;
}

static void
update_control_point_colors (GocItem *item, GtkStateFlags flags)
{
	GtkStyleContext *context = goc_item_get_style_context (item);
	GOStyle *style = go_styled_object_get_style (GO_STYLED_OBJECT (item));
	GdkRGBA rgba;

    MyDragPointPrivate *priv = my_drag_point_get_instance_private(MY_DRAG_POINT(item));

    if(priv->is_dragged)
        return;

	gtk_style_context_get_color (context, flags, &rgba);
	go_color_from_gdk_rgba (&rgba, &style->line.color);
	gtk_style_context_get_background_color (context, flags, &rgba);
	go_color_from_gdk_rgba (&rgba, &style->fill.pattern.back);
	goc_item_invalidate (item);
}

static gboolean
my_drag_point_enter_notify (GocItem *item, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y)
{
    /*update_control_point_colors (item, GTK_STATE_FLAG_PRELIGHT);*/
	return TRUE;
}

static gboolean
my_drag_point_leave_notify (GocItem *item, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y)
{
    /*update_control_point_colors (item, GTK_STATE_FLAG_NORMAL);*/
	return TRUE;
}

static void
my_drag_point_class_init (MyDragPointClass * klass)
{
    GObjectClass *gobject_class;
    GocItemClass *gi_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gi_class = (GocItemClass *) klass;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_drag_point_finalize;
    gobject_class->dispose = my_drag_point_dispose;
    gobject_class->set_property = my_drag_point_set_property;
    gobject_class->get_property = my_drag_point_get_property;

    obj_properties[PROP_LINKED_ITEM] =
        g_param_spec_object ("linked-item",
                             "linked item",
                             "A pointer to the linked item",
                             GOC_TYPE_ITEM, G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);

    gi_class->button_pressed = my_drag_point_button_pressed;
    gi_class->leave_notify = my_drag_point_leave_notify;
    gi_class->enter_notify = my_drag_point_enter_notify;
}

static void
my_drag_point_sync_with_linked_system (MyDragPoint * self,
                                            GParamSpec * pspec, gpointer data)
{
}

static void
my_drag_point_canvas_changed (MyDragPoint * self, GParamSpec * pspec, gpointer data)
{
    GocCanvas *canvas;
    GtkWidget *window;
    GAction *action;
    GVariant *state;

    g_object_get(self, "canvas", &canvas, NULL);

    if(!GOC_IS_CANVAS(canvas)) 
        return;

    window = gtk_widget_get_toplevel(GTK_WIDGET(canvas));

    action =
        g_action_map_lookup_action (G_ACTION_MAP (window), "show-drag-points");

    state = g_action_get_state(action);

    if(state == NULL)
        return;

    goc_item_set_visible(GOC_ITEM(self), g_variant_get_boolean(state));
}

static void
my_drag_point_init (MyDragPoint * self)
{
    MyDragPointPrivate *priv = my_drag_point_get_instance_private(self);
    GOStyle *style;

    /* to init any of the private data, do e.g: */

    priv->linked_item = NULL;
    priv->is_dragged = FALSE;

    style = go_style_dup (go_styled_object_get_style (GO_STYLED_OBJECT(self)));

    style->fill.auto_type = FALSE;
    style->fill.auto_back = FALSE;
    style->fill.type = GO_STYLE_FILL_PATTERN;
    style->fill.pattern.pattern = GO_PATTERN_SOLID;
    style->fill.pattern.back = GO_COLOR_WHITE;
    go_styled_object_set_style (GO_STYLED_OBJECT(self), style);

    g_object_unref (style);

    g_signal_connect(self, "notify::canvas", G_CALLBACK(my_drag_point_canvas_changed), NULL);
}

static void
my_drag_point_dispose (GObject *object)
{
    G_OBJECT_CLASS (my_drag_point_parent_class)->dispose (object);
}

static void
my_drag_point_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_drag_point_parent_class)->finalize (object);
}

/* begin of public functions */

void
my_drag_point_begin_dragging (MyDragPoint * self)
{
    MyDragPointPrivate *priv = my_drag_point_get_instance_private(self);
    GOStyle *style;

    GOStyledObject *gso = GO_STYLED_OBJECT(self);

    g_return_if_fail(MY_IS_DRAG_POINT(self));

    priv->is_dragged = TRUE;

    style = go_style_dup (go_styled_object_get_style (gso));
    style->fill.pattern.back = GO_COLOR_WHITE;
    go_styled_object_set_style (gso, style);

    g_object_unref (style);

    if(MY_IS_FLOW_ARROW(priv->linked_item)) {
        my_flow_arrow_begin_dragging(MY_FLOW_ARROW(priv->linked_item));
    }
}

gboolean
my_drag_point_is_dragged (MyDragPoint * self)
{
    MyDragPointPrivate *priv = my_drag_point_get_instance_private(self);

    g_return_if_fail(MY_IS_DRAG_POINT(self));

    return priv->is_dragged;
}

void
my_drag_point_end_dragging (MyDragPoint * self)
{
    MyDragPointPrivate *priv = my_drag_point_get_instance_private(self);

    g_return_if_fail(MY_IS_DRAG_POINT(self));
    
    if(MY_IS_FLOW_ARROW(priv->linked_item)) {
        my_flow_arrow_end_dragging(MY_FLOW_ARROW(priv->linked_item));
    }

    priv->is_dragged = FALSE;
}
