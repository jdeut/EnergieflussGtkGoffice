#include "my-timelinemodel.h"

#define TIMELINE_MODEL_CURRENT_INDEX        (((gint)gtk_adjustment_get_value (priv->adjust)) - 1)
#define TIMELINE_MODEL_INDEX_IS_TRANSFER    TIMELINE_MODEL_CURRENT_INDEX % 2 == 1

static void
my_timeline_model_time_pos_added (MyTimelineModel * self, guint pos,
                                  gpointer data);

void
tl_systems_data_fill_new_n_at_pos (MyTimelineModel * self, guint pos,
                                   gpointer data);

void
tl_systems_data_new_models_for_appended_element (GPtrArray * tl_systems_data);


guint g_counter = 0;

/* Signals */
enum
{
    SIG_SYSTEMS_CHANGED,
    SIG_SYSTEM_ADDED,
    SIG_SYSTEM_REMOVED,
    SIG_TIME_POS_ADDED,
    SIG_ARROW_ADDED_AT_CURRENT_POS,
    SIG_CURRENT_POS_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0, };

enum
{
    PROP_0,
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
    GPtrArray *tl_arrows;
    GPtrArray *tl_systems_data;
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

    if (gtk_adjustment_get_upper (priv->adjust) > priv->tl_systems_data->len) {
        g_print ("index > array length\n");
    }
    priv->index = TIMELINE_MODEL_CURRENT_INDEX;
}

static void
my_timeline_model_set_property (GObject * object,
                                guint property_id,
                                const GValue * value, GParamSpec * pspec)
{
    MyTimelineModel *self = MY_TIMELINE_MODEL (object);

    switch (property_id) {

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

    signals[SIG_TIME_POS_ADDED] =
        g_signal_new ("time-pos-added",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 1, G_TYPE_UINT);

    signals[SIG_SYSTEM_ADDED] =
        g_signal_new ("system-added",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 1, MY_TYPE_SYSTEM);

    signals[SIG_ARROW_ADDED_AT_CURRENT_POS] =
        g_signal_new ("arrow-added-at-current-pos",
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

    signals[SIG_CURRENT_POS_CHANGED] =
        g_signal_new ("current-pos-changed",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

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

    g_signal_emit (G_OBJECT (self), signals[SIG_CURRENT_POS_CHANGED], 0);
}

static void
my_timeline_model_init (MyTimelineModel * self)
{
    GPtrArray *timeline_first_n;

    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (MY_TIMELINE_MODEL (self));

    priv->systems = g_ptr_array_new ();
    priv->tl_systems_data = g_ptr_array_new ();
    priv->tl_arrows = g_ptr_array_new ();

    g_ptr_array_set_free_func (priv->systems, g_object_unref);
    g_ptr_array_set_free_func (priv->tl_systems_data,
                               (GDestroyNotify) g_ptr_array_unref);
    g_ptr_array_set_free_func (priv->tl_arrows,
                               (GDestroyNotify) g_ptr_array_unref);

    /* The page size is normally 0 for GtkScale */
    priv->adjust =
        gtk_adjustment_new (priv->tl_systems_data->len, 1.0, 1.0, 1.0, 0.5,
                            0.0);

    g_signal_connect_swapped (priv->adjust, "value-changed",
                              G_CALLBACK (my_timeline_index_changed), self);

    g_signal_connect (self, "time-pos-added",
                      G_CALLBACK (tl_systems_data_fill_new_n_at_pos), NULL);

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
    g_ptr_array_unref (priv->tl_systems_data);
    g_ptr_array_unref (priv->tl_arrows);

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

MySystem *
my_timeline_get_system_with_id (MyTimelineModel * self, guint id)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    MySystem *system;

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    system = g_ptr_array_index(priv->systems, id);

    g_return_val_if_fail(MY_IS_SYSTEM(system), NULL);

    return system;
}

GPtrArray *
my_timeline_model_get_systems_data_of_current_pos (MyTimelineModel * self)
{
    GPtrArray *arr;
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    arr = g_ptr_array_index (priv->tl_systems_data,
                             TIMELINE_MODEL_CURRENT_INDEX);

    return arr;
}

GPtrArray *
my_timeline_model_get_arrows_of_current_pos (MyTimelineModel * self)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    if (TIMELINE_MODEL_INDEX_IS_TRANSFER) {

        guint index;

        index = TIMELINE_MODEL_CURRENT_INDEX;

        if (!(index > priv->tl_systems_data->len)) {

            return g_ptr_array_index (priv->tl_arrows, index);
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
                g_ptr_array_index (priv->tl_arrows,
                                   TIMELINE_MODEL_CURRENT_INDEX);

            g_ptr_array_remove (transition, object);

            /*g_signal_emit (G_OBJECT (self), */
            /*signals[SIG_ARROW_REMOVED_AT_CURRENT_INDEX], 0, */
            /*object); */
        }
        else {
            g_print ("can't remove arrow in a state\n");
            return FALSE;
        }
    }

    return TRUE;
}

MySystemModel *
tl_systems_data_get_ith_model_at_pos (GPtrArray * tl_systems_data, guint n,
                                      guint pos)
{
    MySystemModel *model;
    GPtrArray *systems_data_at_pos;

    systems_data_at_pos = g_ptr_array_index (tl_systems_data, pos);

    model = g_ptr_array_index (systems_data_at_pos, n);

    g_return_val_if_fail (MY_IS_SYSTEM_MODEL (model), NULL);

    return model;
}

gboolean
my_timeline_model_add_object (MyTimelineModel * self, gpointer object)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    if (MY_IS_SYSTEM (object)) {

        MySystemWidget *widget;
        MySystemModel *model;
        GPtrArray *n;
        guint i;

        /* add new system-data-model of the system for every instance of time */
        tl_systems_data_new_models_for_appended_element (priv->tl_systems_data);

        g_object_get (object, "widget", &widget, NULL);

        g_return_if_fail (MY_IS_SYSTEM_WIDGET (widget));

        g_ptr_array_add (priv->systems, object);
        /* remember the -1 */
        g_object_set (object, "id", priv->systems->len - 1, NULL);

        model =
            tl_systems_data_get_ith_model_at_pos (priv->tl_systems_data,
                                                  priv->systems->len - 1,
                                                  TIMELINE_MODEL_CURRENT_INDEX);

        g_object_set (widget, "model", model, NULL);

        g_signal_emit (G_OBJECT (self), signals[SIG_SYSTEMS_CHANGED], 0);
        g_signal_emit (G_OBJECT (self), signals[SIG_SYSTEM_ADDED], 0, object);
    }
    else if (MY_IS_FLOW_ARROW (object)) {

        if (TIMELINE_MODEL_INDEX_IS_TRANSFER) {

            GPtrArray *transition;

            transition =
                g_ptr_array_index (priv->tl_arrows,
                                   TIMELINE_MODEL_CURRENT_INDEX);
            g_ptr_array_add (transition, object);
            g_signal_emit (G_OBJECT (self),
                           signals[SIG_ARROW_ADDED_AT_CURRENT_POS], 0, object);
        }
        else {
            g_print ("can't add arrow in a state\n");
            return FALSE;
        }
    }

    return TRUE;
}

/* dimension of time */

void
tl_systems_data_fill_new_n_at_pos (MyTimelineModel * self,
                                   guint pos, gpointer data)
{
    guint i;
    GPtrArray *systems_data_n;
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (MY_TIMELINE_MODEL (self));
    systems_data_n = g_ptr_array_index (priv->tl_systems_data, pos);
    for (i = 0; i < priv->systems->len; i++) {

        MySystemModel *model;

        model = g_object_new (MY_TYPE_SYSTEM_MODEL, NULL);
        g_ptr_array_add (systems_data_n, model);
        g_counter++;
    }
}

void
tl_systems_data_new_models_for_appended_element (GPtrArray * tl_systems_data)
{
    guint i;
    MySystemModel *model;
    GPtrArray *systems_data_n;

    for (i = 0; i < tl_systems_data->len; i++) {

        /* get i'th element of systems_data */
        systems_data_n = g_ptr_array_index (tl_systems_data, i);
        model = g_object_new (MY_TYPE_SYSTEM_MODEL, NULL);
        g_ptr_array_add (systems_data_n, model);
    }
}

static void
g_ptr_array_insert_empty_array_at_pos (GPtrArray * array, gint pos)
{
    GPtrArray *new_element;

    new_element = g_ptr_array_new ();
    g_ptr_array_insert (array, pos, new_element);
}

void
my_timeline_model_add_at_current_pos (MyTimelineModel * self)
{
    guint i;
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);
    for (i = TIMELINE_MODEL_CURRENT_INDEX + 1;
         i < TIMELINE_MODEL_CURRENT_INDEX + 3; i++) {

        g_ptr_array_insert_empty_array_at_pos (priv->tl_systems_data, i);
        g_ptr_array_insert_empty_array_at_pos (priv->tl_arrows, i);
        g_signal_emit (G_OBJECT (self), signals[SIG_TIME_POS_ADDED], 0, i);
    }

    i = gtk_adjustment_get_upper (priv->adjust);
    gtk_adjustment_set_upper (priv->adjust, i + 2);
}

void
my_timeline_model_append_to_timeline (MyTimelineModel * self)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);
    g_ptr_array_insert_empty_array_at_pos (priv->tl_systems_data, -1);
    g_ptr_array_insert_empty_array_at_pos (priv->tl_arrows, -1);
    g_signal_emit (G_OBJECT (self), signals[SIG_TIME_POS_ADDED],
                   0, priv->tl_systems_data->len - 1);
    gtk_adjustment_set_upper (priv->adjust, priv->tl_systems_data->len);
}

gboolean
my_timeline_model_current_pos_is_state (MyTimelineModel * self)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));

    if (TIMELINE_MODEL_CURRENT_INDEX % 2 == 1) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}

guint
my_timeline_model_get_current_pos (MyTimelineModel * self)
{
    MyTimelineModelPrivate *priv =
        my_timeline_model_get_instance_private (self);

    g_return_if_fail (MY_IS_TIMELINE_MODEL (self));
    g_return_if_fail (GTK_IS_ADJUSTMENT (priv->adjust));

    return gtk_adjustment_get_value (priv->adjust);
}
