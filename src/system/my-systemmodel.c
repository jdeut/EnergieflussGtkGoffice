#include "my-systemmodel.h"

/* 'private'/'protected' functions */
static void my_system_model_class_init (MySystemModelClass * klass);
static void my_system_model_init (MySystemModel * self);
static void my_system_model_finalize (GObject * );
static void my_system_model_dispose (GObject * );

struct _MySystemModelPrivate
{
    /* private members go here */
};

G_DEFINE_TYPE_WITH_PRIVATE (MySystemModel, my_system_model, G_TYPE_OBJECT);

GQuark
my_system_model_error_quark (void)
{
  return g_quark_from_static_string ("my-system-model-error-quark");
}

static void
my_system_model_class_init (MySystemModelClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_system_model_finalize;
    gobject_class->dispose = my_system_model_dispose;

    g_type_class_add_private (gobject_class,
                              sizeof (MySystemModelPrivate));
}

static void
my_system_model_init (MySystemModel * self)
{
    MySystemModelPrivate *priv;

    priv = my_system_model_get_instance_private(self);

    /* to init any of the private data, do e.g: */

}

static void
my_system_model_dispose (GObject *object)
{
    G_OBJECT_CLASS (my_system_model_parent_class)->dispose (object);
}

static void
my_system_model_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_system_model_parent_class)->finalize (object);
}

MySystemModel *
my_system_model_new (void)
{
    MySystemModel *self;

    self = g_object_new (MY_TYPE_SYSTEM_MODEL, NULL);

    return self;
}
