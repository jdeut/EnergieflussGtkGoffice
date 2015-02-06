#include "my-intensitybox.h"

/* 'private'/'protected' functions */
static void my_intensity_box_class_init (MyIntensityBoxClass * klass);
static void my_intensity_box_init (MyIntensityBox * self);
static void my_intensity_box_finalize (GObject *);
static void my_intensity_box_dispose (GObject *);

struct _MyIntensityBoxPrivate
{
    /* private members go here */
    gint i;
};

G_DEFINE_TYPE_WITH_PRIVATE (MyIntensityBox, my_intensity_box,
                            GTK_TYPE_DRAWING_AREA);

GQuark
my_intensity_box_error_quark (void)
{
    return g_quark_from_static_string ("my-intensity-box-error-quark");
}

gboolean
my_intensity_box_draw (GtkWidget * widget, cairo_t * cr)
{

    gint x, y;
    gint width, height;

    x = 4;
    y = 0;

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    cairo_rectangle (cr, 10, 20, width-20, height-8-20);
    cairo_set_source_rgba (cr, 1, 0, 0, 0.5);
    cairo_fill (cr);

    cairo_set_line_width (cr, 2);
    cairo_set_source_rgba (cr, 0, 0, 0, 0.8);
    cairo_move_to (cr, x, height-8);
    cairo_rel_line_to (cr, width, 0);
    cairo_stroke (cr);

    cairo_set_line_width (cr, 0.8);
    cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 0.8);
    cairo_move_to (cr, x, height-8);
    cairo_rel_line_to (cr, 0, -height-8);
    cairo_stroke (cr);

    return TRUE;
}

static GtkSizeRequestMode
my_intensity_box_get_request_mode (GtkWidget * widget)
{
    return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}

static void
my_intensity_box_get_preferred_size (GtkWidget * widget,
                                     GtkOrientation orientation,
                                     gint * minimum_size,
                                     gint * natural_size)
{
    if (orientation = GTK_ORIENTATION_HORIZONTAL) {
        *minimum_size = 5;
        *natural_size = 10;
    } else {
        *minimum_size = 20;
        *natural_size = 40;
    }
}

static void
my_intensity_box_get_preferred_width (GtkWidget * widget,
                                      gint * minimum_size, gint * natural_size)
{
    my_intensity_box_get_preferred_size (widget, GTK_ORIENTATION_HORIZONTAL,
                                         minimum_size, natural_size);
}

static void
my_intensity_box_get_preferred_height (GtkWidget * widget,
                                       gint * minimum_size, gint * natural_size)
{
    my_intensity_box_get_preferred_size (widget, GTK_ORIENTATION_VERTICAL,
                                         minimum_size, natural_size);
}

static void
my_intensity_box_get_preferred_width_for_height (GtkWidget * widget,
                                                 gint height,
                                                 gint * minimum_width,
                                                 gint * natural_width)
{
    *minimum_width = height/10;
    *natural_width = height/5;
}

static void
my_intensity_box_get_preferred_height_for_width (GtkWidget * widget,
                                                 gint width,
                                                 gint * minimum_height,
                                                 gint * natural_height)
{
    *minimum_height = 10;
    *natural_height = 30;
}

static void
my_intensity_box_class_init (MyIntensityBoxClass * klass)
{
    GObjectClass *gobject_class;
    GtkWidgetClass *widget_class;

    gobject_class = G_OBJECT_CLASS (klass);
    widget_class = (GtkWidgetClass *) klass;

    gobject_class->finalize = my_intensity_box_finalize;
    gobject_class->dispose = my_intensity_box_dispose;

    widget_class->draw = my_intensity_box_draw;

    widget_class->get_request_mode = my_intensity_box_get_request_mode;
    widget_class->get_preferred_width = my_intensity_box_get_preferred_width;
    widget_class->get_preferred_height = my_intensity_box_get_preferred_height;
    widget_class->get_preferred_width_for_height =
        my_intensity_box_get_preferred_width_for_height;
    widget_class->get_preferred_height_for_width =
        my_intensity_box_get_preferred_height_for_width;
}

static void
my_intensity_box_init (MyIntensityBox * self)
{
    MyIntensityBoxPrivate *priv;

    priv = my_intensity_box_get_instance_private (self);

    gtk_widget_set_halign(GTK_WIDGET(self), GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(GTK_WIDGET(self), TRUE);

    /* to init any of the private data, do e.g: */

}

static void
my_intensity_box_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_intensity_box_parent_class)->dispose (object);
}

static void
my_intensity_box_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_intensity_box_parent_class)->finalize (object);
}

MyIntensityBox *
my_intensity_box_new (void)
{
    MyIntensityBox *self;

    self = g_object_new (MY_TYPE_INTENSITY_BOX, NULL);

    return self;
}
