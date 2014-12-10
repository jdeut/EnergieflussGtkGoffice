#include "my-system.h"

/* 'private'/'protected' functions */
static void my_system_class_init (MySystemClass * klass);
static void my_system_init (MySystem * self);
static void my_system_finalize (GObject * );
static void my_system_dispose (GObject * );

struct _MySystemPrivate
{
    /* private members go here */
};

#define MY_SYSTEM_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                       MY_TYPE_SYSTEM, \
                                       MySystemPrivate))

G_DEFINE_TYPE (MySystem, my_system, GOC_TYPE_WIDGET);


GQuark
my_system_error_quark (void)
{
  return g_quark_from_static_string ("my-system-error-quark");
}

static void
my_system_class_init (MySystemClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_system_finalize;
    gobject_class->dispose = my_system_dispose;

    g_type_class_add_private (gobject_class,
                              sizeof (MySystemPrivate));
}

static void
my_system_init (MySystem * self)
{
    self->_priv = MY_SYSTEM_GET_PRIVATE (self);

    /* to init any of the private data, do e.g: */

    /* 	obj->_priv->_frobnicate_mode = FALSE; */
}

static void
my_system_dispose (GObject *object)
{
    G_OBJECT_CLASS (my_system_parent_class)->dispose (object);
}

static void
my_system_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_system_parent_class)->finalize (object);
}

MySystem *
my_system_new (void)
{
    MySystem *self;

    self = g_object_new (MY_TYPE_SYSTEM, NULL);

    return self;
}
