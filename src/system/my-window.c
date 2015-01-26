#include "my-window.h"

/* 'private'/'protected' functions */
static void my_window_class_init (MyWindowClass * klass);
static void my_window_init (MyWindow * self);
static void my_window_finalize (GObject * );
static void my_window_dispose (GObject * );

struct _MyWindowPrivate
{
    /* private members go here */
};

G_DEFINE_TYPE_WITH_PRIVATE (MyWindow, my_window, GTK_TYPE_APPLICATION_WINDOW);

GQuark
my_window_error_quark (void)
{
  return g_quark_from_static_string ("my-window-error-quark");
}

static void
my_window_constructed (GObject *object) {
}

static void
my_window_class_init (MyWindowClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_window_finalize;
    gobject_class->dispose = my_window_dispose;
    gobject_class->constructed = my_window_constructed;

    g_type_class_add_private (gobject_class,
                              sizeof (MyWindowPrivate));
}

static void
my_window_init (MyWindow * self)
{
    MyWindowPrivate *priv;

    priv = my_window_get_instance_private(self);

    /* to init any of the private data, do e.g: */

}

static void
my_window_dispose (GObject *object)
{
    G_OBJECT_CLASS (my_window_parent_class)->dispose (object);
}

static void
my_window_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_window_parent_class)->finalize (object);
}

MyWindow *
my_window_new (void)
{
    MyWindow *self;

    self = g_object_new (MY_TYPE_WINDOW, NULL);

    return self;
}
