#include "my-timelinemodel.h"

#define TIMELINE_MODEL_CURRENT_INDEX        (((gint)gtk_adjustment_get_value (priv->adjust)) - 1)
#define TIMELINE_MODEL_INDEX_IS_TRANSFER    TIMELINE_MODEL_CURRENT_INDEX % 2 == 1

/* Signals */
enum
{
    SIG_CHANGED,
    SIG_SYSTEMS_CHANGED,
    SIG_SYSTEM_ADDED,
    SIG_SYSTEM_REMOVED,
    SIG_ARROW_ADDED_AT_CURRENT_INDEX,
    SIG_CURRENT_INDEX_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0, };

enum
{
    PROP_0,
    /* property entries */
    PROP_INDEX,
    PROP_INDEX_MAX,
    PROP_ADJUST,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* 'private'/'protected' functions */
static void my_timeline_model_class_init (MyTimelineModelClass * klass);
static void my_timeline_model_init (MyTimelineModel * self);
static void my_timeline_model_finalize (GObject *);
static void my_timeline_model_dispose (GObject *);

struct _MyTimelineModelPrivate
{
    guint index;
    GPtrArray *systems;
    GPtrArray *timeline;
    GtkAdjustment *adjust;
};


G_DEFINE_TYPE_WITH_PRIVATE (MyTimelineModel, my_timeline_model, G_TYPE_OBJECT);


GQuark
my_timeline_model_error_quark (void)
{
    return g_quark_from_static_string ("my-timeline-model-error-quark");
}

static void
my_timeline_model_set_index (MyTimelineModel * self, guint index)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    if (index > priv->timeline->len) {
        g_print ("index > array length\n");
    }
    priv->index = index;
}


static void
my_timeline_model_set_property (GObject * object,
                                guint property_id,
                                const GValue * value, GParamSpec * pspec)
{
    MyTimelineModel *self = MY_TIMELINE_MODEL (object);

    switch (property_id) {

        case PROP_INDEX:
            my_timeline_model_set_index (self, g_value_get_uint (value));
            break;

        case PROP_ADJUST:
            self->_priv->adjust = g_value_get_object (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_timeline_model_get_property (GObject * object,
                                guint property_id,
                                GValue * value, GParamSpec * pspec)
{

    MyTimelineModel *self = MY_TIMELINE_MODEL (object);

    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (MY_TIMELINE_MODEL (self));

    switch (property_id) {

        case PROP_INDEX:
            g_value_set_uint (value, priv->index);
            break;

        case PROP_INDEX_MAX:
            g_value_set_uint (value, priv->timeline->len);
            break;

        case PROP_ADJUST:
            g_value_set_object (value, priv->adjust);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_timeline_model_class_init (MyTimelineModelClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_timeline_model_finalize;
    gobject_class->dispose = my_timeline_model_dispose;
    gobject_class->set_property = my_timeline_model_set_property;
    gobject_class->get_property = my_timeline_model_get_property;

    signals[SIG_CHANGED] =
        g_signal_new ("changed",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[SIG_SYSTEM_ADDED] =
        g_signal_new ("system-added",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 1, MY_TYPE_SYSTEM);

    signals[SIG_ARROW_ADDED_AT_CURRENT_INDEX] =
        g_signal_new ("arrow-added-at-current-index",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 1, MY_TYPE_FLOW_ARROW);

    signals[SIG_SYSTEM_REMOVED] =
        g_signal_new ("system-removed",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[SIG_SYSTEMS_CHANGED] =
        g_signal_new ("systems-changed",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    signals[SIG_CURRENT_INDEX_CHANGED] =
        g_signal_new ("current-index-changed",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    obj_properties[PROP_INDEX] =
        g_param_spec_uint ("index",
                           "index",
                           "Defines the current state/transfer",
                           1, G_MAXUINT, 1,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    obj_properties[PROP_INDEX_MAX] =
        g_param_spec_uint ("index-max",
                           "index-max",
                           "length of timeline array",
                           1, G_MAXUINT, 1, G_PARAM_READABLE);

    obj_properties[PROP_ADJUST] =
        g_param_spec_object ("adjustment",
                             "adjustment",
                             "adjustment",
                             GTK_TYPE_ADJUSTMENT, G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);
}

void
my_timeline_index_changed (MyTimelineModel * self, GtkAdjustment * adjust)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (MY_TIMELINE_MODEL (self));

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));
    g_return_if_fail (GTK_IS_ADJUSTMENT (adjust));


    g_print ("value: %u\n", TIMELINE_MODEL_CURRENT_INDEX);

    g_signal_emit (G_OBJECT (self), signals[SIG_CURRENT_INDEX_CHANGED], 0);
}

static void
my_timeline_model_init (MyTimelineModel * self)
{
    GPtrArray *timeline_first_n;

    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (MY_TIMELINE_MODEL (self));

    priv->systems = g_ptr_array_new ();
    priv->timeline = g_ptr_array_new ();

    g_ptr_array_set_free_func (priv->systems, g_object_unref);
    g_ptr_array_set_free_func (priv->timeline,
                               (GDestroyNotify) g_ptr_array_unref);

    /* The page size is normally 0 for GtkScale */
    priv->adjust =
        gtk_adjustment_new (priv->timeline->len, 1.0, 1.0, 1.0, 0.5, 0.0);

    g_signal_connect_swapped (priv->adjust, "value-changed",
                              G_CALLBACK (my_timeline_index_changed), self);

    my_timeline_model_append_to_timeline (self);
}

static void
my_timeline_model_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_timeline_model_parent_class)->dispose (object);
}

static void
my_timeline_model_finalize (GObject * object)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (MY_TIMELINE_MODEL (object));

    /* remember to set clear func */
    g_ptr_array_unref (priv->systems);
    g_ptr_array_unref (priv->timeline);

    g_object_unref (priv->adjust);

    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_timeline_model_parent_class)->finalize (object);
}

MyTimelineModel *
my_timeline_model_new (void)
{
    MyTimelineModel *self;

    self = g_object_new (MY_TYPE_TIMELINE_MODEL, NULL);

    return self;
}

/* public methods */

GPtrArray *
my_timeline_get_systems (MyTimelineModel * self)
{

    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    return priv->systems;
}

GPtrArray *
my_timeline_model_get_arrows_of_current_index (MyTimelineModel * self)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    if (TIMELINE_MODEL_INDEX_IS_TRANSFER) {

        guint index;

        index = TIMELINE_MODEL_CURRENT_INDEX;

        if (!(index > priv->timeline->len)) {

            return g_ptr_array_index (priv->timeline, index);
        }
    }

    return NULL;
}

gboolean
my_timeline_model_remove_object (MyTimelineModel * self, gpointer object)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    if (MY_IS_SYSTEM (object)) {
    }
    else if (MY_IS_FLOW_ARROW (object)) {

        if (TIMELINE_MODEL_INDEX_IS_TRANSFER) {

            GPtrArray *transition;

            transition =
                g_ptr_array_index (priv->timeline,
                                   TIMELINE_MODEL_CURRENT_INDEX);

            g_ptr_array_remove (transition, object);

            /*g_signal_emit (G_OBJECT (self),*/
                           /*signals[SIG_ARROW_REMOVED_AT_CURRENT_INDEX], 0,*/
                           /*object);*/
        }
        else {
            g_print ("can't remove arrow in a state\n");
            return FALSE;
        }
    }

    g_signal_emit (G_OBJECT (self), signals[SIG_CHANGED], 0);

    return TRUE;
}

gboolean
my_timeline_model_add_object (MyTimelineModel * self, gpointer object)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    if (MY_IS_SYSTEM (object)) {
        g_ptr_array_add (priv->systems, object);

        g_signal_emit (G_OBJECT (self), signals[SIG_SYSTEMS_CHANGED], 0);
        g_signal_emit (G_OBJECT (self), signals[SIG_SYSTEM_ADDED], 0, object);
    }
    else if (MY_IS_FLOW_ARROW (object)) {

        if (TIMELINE_MODEL_INDEX_IS_TRANSFER) {

            GPtrArray *transition;

            transition =
                g_ptr_array_index (priv->timeline,
                                   TIMELINE_MODEL_CURRENT_INDEX);

            g_ptr_array_add (transition, object);

            g_signal_emit (G_OBJECT (self),
                           signals[SIG_ARROW_ADDED_AT_CURRENT_INDEX], 0,
                           object);
        }
        else {
            g_print ("can't add arrow in a state\n");
            return FALSE;
        }
    }

    g_signal_emit (G_OBJECT (self), signals[SIG_CHANGED], 0);

    return TRUE;
}

void
my_timeline_model_append_to_timeline (MyTimelineModel * self)
{

    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    GPtrArray *new_element;

    new_element = g_ptr_array_new ();

    g_ptr_array_add (priv->timeline, new_element);

    gtk_adjustment_set_upper (priv->adjust, priv->timeline->len);
}
