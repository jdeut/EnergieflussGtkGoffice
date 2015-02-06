#include "my-canvas.h"

#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

enum
{
    PROP_0,
    /* property entries */
    PROP_TIMELINE,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

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
    MyTimelineModel *timeline;
};

/* prototypes of private methods */

void
my_canvas_group_add_item (MyCanvas * self, GocGroup * group, GocItem * item);


G_DEFINE_TYPE_WITH_PRIVATE (MyCanvas, my_canvas, GOC_TYPE_CANVAS);


static void
my_canvas_set_property (GObject * object,
                        guint property_id,
                        const GValue * value, GParamSpec * pspec)
{
    MyCanvas *self = MY_CANVAS (object);

    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    switch (property_id) {

        case PROP_TIMELINE:
            priv->timeline = g_value_get_object (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_canvas_get_property (GObject * object,
                        guint property_id, GValue * value, GParamSpec * pspec)
{

    MyCanvas *self = MY_CANVAS (object);

    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    switch (property_id) {

        case PROP_TIMELINE:
            g_value_set_object (value, priv->timeline);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

GQuark
my_canvas_error_quark (void)
{
    return g_quark_from_static_string ("my-canvas-error-quark");
}

static gboolean
my_canvas_drag_drag_point (MyCanvas * self, gdouble x, gdouble y)
{
    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    g_return_if_fail (MY_IS_DRAG_POINT (priv->active_item));

    goc_item_set (priv->active_item, "x", x, "y", y, NULL);
}

static void
my_canvas_class_init (MyCanvasClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_canvas_finalize;
    gobject_class->dispose = my_canvas_dispose;
    gobject_class->set_property = my_canvas_set_property;
    gobject_class->get_property = my_canvas_get_property;

    obj_properties[PROP_TIMELINE] =
        g_param_spec_object ("timeline",
                             "timeline",
                             "Timeline",
                             MY_TYPE_TIMELINE_MODEL, G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);
}

static void
my_canvas_init (MyCanvas * self)
{
    GocGroup *root;
    guint i;

    MyCanvasPrivate *priv = my_canvas_get_instance_private (self);

    root = goc_canvas_get_root (GOC_CANVAS (self));

    priv = my_canvas_get_instance_private (self);

    priv->active_item = NULL;
    priv->add_arrow_mode = FALSE;
    priv->add_system_mode = FALSE;

    for (i = 0; i <= N_GROUPS; i++) {
        self->group[i] = goc_group_new (root);
    }

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

    group = self->group[GROUP_ARROWS];

    for (l = group->children; l != NULL; l = l->next) {
        if (MY_IS_FLOW_ARROW (l->data)) {
            my_flow_arrow_show_drag_points (MY_FLOW_ARROW (l->data));
        }
    }
}

void
my_canvas_hide_drag_points_of_all_arrows (MyCanvas * self)
{
    GList *l;
    GocGroup *group;

    group = self->group[GROUP_ARROWS];

    for (l = group->children; l != NULL; l = l->next) {
        if (MY_IS_FLOW_ARROW (l->data)) {
            my_flow_arrow_hide_drag_points (MY_FLOW_ARROW (l->data));
        }
    }
}

gboolean
my_canvas_generate_json_data_stream (MyCanvas * self, gchar ** str, gsize * len)
{
    JsonGenerator *gen = json_generator_new ();
    JsonArray *array_arrows, *array_systems, *array_root;
    JsonNode *node, *root, *arrows, *systems;
    GocGroup *group_arrows, *group_systems;
    GList *l;

    group_arrows = self->group[GROUP_ARROWS];
    group_systems = self->group[GROUP_SYSTEMS];

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
my_canvas_transform_coordinate (GocCanvas * canvas, gdouble * x, gdouble * y)
{

    if (x != NULL) {
        *x = (canvas->direction ==
              GOC_DIRECTION_RTL) ? canvas->scroll_x1 + (canvas->width -
                                                        *x) /
            canvas->pixels_per_unit : canvas->scroll_x1 +
            *x / canvas->pixels_per_unit;
    }
    if (y != NULL) {
        *y = canvas->scroll_y1 + *y / canvas->pixels_per_unit;
    }
}

static gboolean
my_canvas_button_press_1_cb (GocCanvas * canvas, GdkEventButton * event,
                             gpointer data)
{
    MyCanvas *self = MY_CANVAS (canvas);

    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    gdouble offsetx, offsety;
    gdouble x_cv, y_cv, dx, dy;

    /* coordinates on canvas */

    gdk_window_get_device_position_double (gtk_widget_get_window
                                           (GTK_WIDGET (canvas)), event->device,
                                           &x_cv, &y_cv, NULL);

    my_canvas_transform_coordinate (canvas, &x_cv, &y_cv);

    g_print ("x_cv: %f, y_cv: %f\n", x_cv, y_cv);
    g_print ("eventx: %f, eventy: %f\n", event->x, event->y);

    priv->active_item = goc_canvas_get_item_at (canvas, x_cv, y_cv);

    /* if in ADD SYSTEM MODE */
    if (priv->add_system_mode && !GOC_IS_ITEM (priv->active_item)) {

        MySystem *system;

        system = g_object_new (MY_TYPE_SYSTEM, "x", x_cv, "y", y_cv, NULL);

        my_timeline_model_add_object (priv->timeline, system);

        priv->add_system_mode = FALSE;
    }
    /* if in ADD ARROW MODE */
    else if (priv->add_arrow_mode && MY_IS_SYSTEM (priv->active_item)) {

        MyFlowArrow *arrow;
        MyDragPoint *point;

        arrow =
            g_object_new (MY_TYPE_FLOW_ARROW, "primary-system",
                          priv->active_item, "x1", x_cv, "y1", y_cv,
                          "x0", x_cv, "y0", y_cv, NULL);

        if (!my_timeline_model_add_object (priv->timeline, arrow)) {

            priv->add_arrow_mode = FALSE;

            g_object_unref (arrow);

        }
        else {

            my_flow_arrow_show_drag_points (arrow);

            point = my_flow_arrow_get_drag_point (arrow);

            goc_item_set (GOC_ITEM (point), "x", x_cv, "y", y_cv, NULL);

            priv->active_item = GOC_ITEM (point);
        }
    }
    else {
        priv->add_arrow_mode = FALSE;
    }

    /* set offset for different items */
    if (GOC_IS_CIRCLE (priv->active_item)
        || MY_IS_DRAG_POINT (priv->active_item)) {

        gdouble x, y;

        g_object_get (priv->active_item, "x", &x, "y", &y, NULL);

        dx = x_cv - x;
        dy = y_cv - y;

    }
    else if (MY_IS_SYSTEM (priv->active_item)) {

        GtkWidget *widget;

        g_object_get (priv->active_item, "widget", &widget, NULL);

        gdk_window_get_device_position_double (gtk_widget_get_window
                                               (GTK_WIDGET (widget)),
                                               event->device, &dx, &dy, NULL);

        g_object_unref(widget);
    }

    priv->offsetx = dx;
    priv->offsety = dy;

    if (MY_IS_DRAG_POINT (priv->active_item)) {
        my_drag_point_begin_dragging (MY_DRAG_POINT (priv->active_item));
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
    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    MySystem *primary_system;
    MyFlowArrow *arrow;
    GocItem *item;
    gdouble x_cv, y_cv;

    gdk_window_get_device_position_double (gtk_widget_get_window
                                           (GTK_WIDGET (canvas)),
                                           event->button.device, &x_cv, &y_cv,
                                           NULL);

    my_canvas_transform_coordinate (canvas, &x_cv, &y_cv);

    if (priv->add_arrow_mode) {
        priv->add_arrow_mode = FALSE;
    }

    if (MY_IS_DRAG_POINT (priv->active_item)) {

        gdouble d =
            goc_item_distance (GOC_ITEM (self->group[GROUP_SYSTEMS]), x_cv,
                               y_cv, &item);

        g_object_get (priv->active_item, "linked-item", &arrow, NULL);
        g_object_unref (arrow);

        if (MY_IS_FLOW_ARROW (arrow)) {

            g_object_get (arrow, "primary-system", &primary_system, NULL);
            g_object_unref (primary_system);

            /* only do it if drag point is over a system but not over the system the corresponding arrow is linked with */

            if (d == 0. && MY_IS_SYSTEM (item)
                && MY_SYSTEM (primary_system) != MY_SYSTEM (item)) {

                g_object_set (arrow, "secondary-system", item, NULL);

            }
            else {
                g_object_set (arrow, "secondary-system", NULL, NULL);
            }
        }

        my_drag_point_end_dragging (MY_DRAG_POINT (priv->active_item));
    }

    priv->active_item = NULL;

    return TRUE;
}

gboolean
my_canvas_motion_notify_cb (GocCanvas * canvas, GdkEventMotion * event,
                            gpointer data)
{
    MyCanvas *self = MY_CANVAS (canvas);
    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    GocItem *active_item = priv->active_item;

    gdouble x_item_new, y_item_new;

    gdouble x_cv, y_cv;

    gdk_window_get_device_position_double (gtk_widget_get_window
                                           (GTK_WIDGET (canvas)), event->device,
                                           &x_cv, &y_cv, NULL);

    if (active_item == NULL)
        return;

    my_canvas_transform_coordinate (canvas, &x_cv, &y_cv);

    x_item_new = x_cv - priv->offsetx;
    y_item_new = y_cv - priv->offsety;

    if (GOC_IS_WIDGET (active_item)) {
        goc_item_set (active_item, "x", x_item_new, "y", y_item_new, NULL);
    }
    else if (MY_IS_DRAG_POINT (active_item)) {
        goc_item_set (active_item, "x", x_item_new, "y", y_item_new, NULL);
    }
    else if (GOC_IS_CIRCLE (active_item)) {
        goc_item_set (active_item, "x", x_item_new, "y", y_item_new, NULL);
    }

    goc_item_invalidate (active_item);

    gtk_widget_queue_draw (GTK_WIDGET (canvas));

    return TRUE;
}

/* transition */
void
my_canvas_model_current_pos_changed (MyCanvas * self, MyTimelineModel * model)
{
    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    GPtrArray *array;
    GocGroup *root;
    GList *l;
    guint i;

    g_return_if_fail (MY_IS_CANVAS (self));
    g_return_if_fail (MY_IS_TIMELINE_MODEL (model));

    root = goc_canvas_get_root (GOC_CANVAS (self));

    for (i = 0; i <= N_GROUPS; i++) {

        if (i == GROUP_SYSTEMS) {
            continue;

        }
        else if (i == GROUP_ARROWS && GOC_IS_GROUP (self->group[i])) {

            l = self->group[i]->children;

            while (l != NULL) {

                goc_group_remove_child (self->group[i], GOC_ITEM (l->data));

                /* since list changes upon goc_group_remove_child l must be refreshed */
                l = self->group[i]->children;
            }
        }

        goc_item_destroy (GOC_ITEM (self->group[i]));

        self->group[i] = goc_group_new (root);
    }

    goc_item_lower_to_bottom (GOC_ITEM (self->group[GROUP_LABELS]));
    goc_item_lower_to_bottom (GOC_ITEM (self->group[GROUP_ARROWS]));

    array = my_timeline_model_get_arrows_of_current_pos (priv->timeline);

    if (array != NULL) {

        for (i = 0; i < array->len; i++) {

            MyFlowArrow *arrow;

            arrow = g_ptr_array_index (array, i);

            if (MY_IS_FLOW_ARROW (arrow)) {
                my_canvas_group_add_item (NULL, self->group[GROUP_ARROWS],
                                          GOC_ITEM (arrow));
            }
        }
    }
}

void
my_canvas_group_add_item (MyCanvas * self, GocGroup * group, GocItem * item)
{
    g_return_if_fail (GOC_IS_GROUP (group));
    g_return_if_fail (GOC_IS_ITEM (item));

    goc_group_add_child (group, item);
    goc_item_invalidate (item);
}

void
my_canvas_set_add_system_mode (MyCanvas * self)
{
    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    g_return_if_fail (MY_IS_CANVAS (self));

    priv->add_system_mode = TRUE;
}

void
my_canvas_set_add_arrow_mode (MyCanvas * self)
{
    MyCanvasPrivate *priv = my_canvas_get_instance_private (MY_CANVAS (self));

    g_return_if_fail (MY_IS_CANVAS (self));

    priv->add_arrow_mode = TRUE;
}

void
my_canvas_model_arrow_added_at_current_index (MyCanvas * self,
                                              MyFlowArrow * arrow,
                                              MyTimelineModel * model)
{
    g_return_if_fail (MY_IS_CANVAS (self));
    g_return_if_fail (MY_IS_TIMELINE_MODEL (model));
    g_return_if_fail (MY_IS_FLOW_ARROW (arrow));

    my_canvas_group_add_item (self, self->group[GROUP_ARROWS],
                              GOC_ITEM (arrow));
}

void
my_canvas_model_system_added (MyCanvas * self, MySystem * system,
                              MyTimelineModel * model)
{
    g_return_if_fail (MY_IS_CANVAS (self));
    g_return_if_fail (MY_IS_TIMELINE_MODEL (model));
    g_return_if_fail (MY_IS_SYSTEM (system));

    my_canvas_group_add_item (self, self->group[GROUP_SYSTEMS],
                              GOC_ITEM (system));
}

void
my_canvas_model_systems_changed (MyCanvas * self, MyTimelineModel * model)
{
    g_return_if_fail (MY_IS_CANVAS (self));
    g_return_if_fail (MY_IS_TIMELINE_MODEL (model));
}

void
my_canvas_set_timeline (MyCanvas * self, MyTimelineModel * model)
{

    g_return_if_fail (MY_IS_CANVAS (self));
    g_return_if_fail (MY_IS_TIMELINE_MODEL (model));

    g_object_set (self, "timeline", model, NULL);

    g_signal_connect_swapped (model, "current-pos-changed",
                              G_CALLBACK
                              (my_canvas_model_current_pos_changed), self);

    g_signal_connect_swapped (model, "systems-changed",
                              G_CALLBACK
                              (my_canvas_model_systems_changed), self);

    g_signal_connect_swapped (model, "system-added",
                              G_CALLBACK (my_canvas_model_system_added), self);

    g_signal_connect_swapped (model, "arrow-added-at-current-pos",
                              G_CALLBACK
                              (my_canvas_model_arrow_added_at_current_index),
                              self);
}
