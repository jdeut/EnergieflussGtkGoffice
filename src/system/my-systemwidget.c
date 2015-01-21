#include "my-systemwidget.h"

/* 'private'/'protected' functions */
static void my_system_widget_class_init (MySystemWidgetClass * klass);
static void my_system_widget_init (MySystemWidget * self);
static void my_system_widget_finalize (GObject *);
static void my_system_widget_dispose (GObject *);

struct _MySystemWidgetPrivate
{
    /* private members go here */
    GtkImage *image;
    GdkPixbuf *pixbuf;
};

#define MY_SYSTEM_WIDGET_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                       MY_TYPE_SYSTEM_WIDGET, \
                                       MySystemWidgetPrivate))

G_DEFINE_TYPE_WITH_PRIVATE (MySystemWidget, my_system_widget, GTK_TYPE_EVENT_BOX);


GQuark
my_system_widget_error_quark (void)
{
    return g_quark_from_static_string ("my-system-widget-error-quark");
}

static void
my_system_widget_class_init (MySystemWidgetClass * klass)
{
    GObjectClass *gobject_class;
    GtkWidgetClass *widget_class;

    gobject_class = G_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->finalize = my_system_widget_finalize;
    gobject_class->dispose = my_system_widget_dispose;

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gtk/myapp/my-system-widget.ui");

    gtk_widget_class_bind_template_child_private (widget_class, MySystemWidget,
                                                  image);
}

static void
my_system_widget_init (MySystemWidget * self)
{
    self->_priv = MY_SYSTEM_WIDGET_GET_PRIVATE (self);

    /* to init any of the private data, do e.g: */

    /*  obj->_priv->_frobnicate_mode = FALSE; */
    gtk_widget_init_template (GTK_WIDGET (self));

}

static void
my_system_widget_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_system_widget_parent_class)->dispose (object);
}

static void
my_system_widget_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_system_widget_parent_class)->finalize (object);
}

void my_system_widget_size_allocate_cb(GtkWidget *widget, GdkRectangle *allocation, gpointer data) {
    g_print("ok\n");
}

MySystemWidget *
my_system_widget_new (void)
{ 
    MySystemWidget *self;

    self = g_object_new (MY_TYPE_SYSTEM_WIDGET, NULL);

    g_signal_connect(self, "size-allocate", G_CALLBACK(my_system_widget_size_allocate_cb), NULL);

    self->_priv->pixbuf = gdk_pixbuf_new_from_resource_at_scale ("/org/gtk/myapp/lagerfeuer.png", 200, -1, TRUE, NULL);

    gtk_image_set_from_pixbuf(self->_priv->image, self->_priv->pixbuf);

    return self;
}
