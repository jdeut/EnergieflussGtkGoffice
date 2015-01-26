#include "my-timelinemodel.h"

/* 'private'/'protected' functions */
static void my_timeline_model_class_init (MyTimelineModelClass * klass);
static void my_timeline_model_init (MyTimelineModel * self);
static void my_timeline_model_finalize (GObject * );
static void my_timeline_model_dispose (GObject * );

struct _MyTimelineModelPrivate
{
    GArray *systems;
    GArray *timeline;
    guint current_state;
};


G_DEFINE_TYPE_WITH_PRIVATE (MyTimelineModel, my_timeline_model, G_TYPE_OBJECT);


GQuark
my_timeline_model_error_quark (void)
{
  return g_quark_from_static_string ("my-timeline-model-error-quark");
}

static void
my_timeline_model_class_init (MyTimelineModelClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_timeline_model_finalize;
    gobject_class->dispose = my_timeline_model_dispose;
}

static void
my_timeline_model_init (MyTimelineModel * self)
{
    MyTimelineModelPrivate *priv = my_timeline_model_get_instance_private(MY_TIMELINE_MODEL(self));

    priv->systems = g_array_new(TRUE, TRUE, sizeof(gpointer));
    priv->timeline = g_array_new(TRUE, TRUE, sizeof(gpointer));

    g_array_set_clear_func(priv->systems, g_object_unref);
    g_array_set_clear_func(priv->timeline, (GDestroyNotify) g_array_unref);
}

static void
my_timeline_model_dispose (GObject *object)
{
    G_OBJECT_CLASS (my_timeline_model_parent_class)->dispose (object);
}

static void
my_timeline_model_finalize (GObject * object)
{
    MyTimelineModelPrivate *priv = my_timeline_model_get_instance_private(MY_TIMELINE_MODEL(object));

    /* remember to set clear func */
    g_array_unref(priv->systems);
    g_array_unref(priv->timeline);

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

void my_timeline_model_add_system(MyTimelineModel *self, MySystem *system) {

    MyTimelineModelPrivate *priv = my_timeline_model_get_instance_private(self);

    g_return_if_fail(MY_IS_TIMELINE_MODEL(self));

    g_array_append_val (priv->systems, system);
}
