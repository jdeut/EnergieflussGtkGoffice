#include "my-intensitybox.h"

/* 'private'/'protected' functions */
static void my_intensity_box_class_init (MyIntensityBoxClass * klass);
static void my_intensity_box_init (MyIntensityBox * self);
static void my_intensity_box_finalize (GObject *);
static void my_intensity_box_dispose (GObject *);

enum
{
    PROP_0,
    PROP_DELTA_ENERGY,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

struct _MyIntensityBoxPrivate
{
    GtkWidget *popover;
    gint i;

    gdouble delta_e;
};

G_DEFINE_TYPE_WITH_PRIVATE (MyIntensityBox, my_intensity_box,
                            GTK_TYPE_DRAWING_AREA);

GQuark
my_intensity_box_error_quark (void)
{
    return g_quark_from_static_string ("my-intensity-box-error-quark");
}

static void
my_intensity_box_set_property (GObject * object,
                               guint property_id,
                               const GValue * value, GParamSpec * pspec)
{
    MyIntensityBox *self = MY_INTENSITY_BOX (object);

    MyIntensityBoxPrivate *priv = my_intensity_box_get_instance_private (self);

    switch (property_id) {

        case PROP_DELTA_ENERGY:
            priv->delta_e = g_value_get_double (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_intensity_box_get_property (GObject * object,
                               guint property_id, GValue * value,
                               GParamSpec * pspec)
{
    MyIntensityBox *self = MY_INTENSITY_BOX (object);

    MyIntensityBoxPrivate *priv = my_intensity_box_get_instance_private (self);

    switch (property_id) {

        case PROP_DELTA_ENERGY:
            g_value_set_double (value, priv->delta_e);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

gboolean
my_intensity_box_draw (GtkWidget * widget, cairo_t * cr)
{
    MyIntensityBox *self = MY_INTENSITY_BOX (widget);

    MyIntensityBoxPrivate *priv = my_intensity_box_get_instance_private (self);

    gint x, y, xc, yc;
    gint width, height, arrow_h, arrow_w, arrow_tip_w, arrow_tip_h;

    gdouble line_width = 2;

    gdouble cur;
    gdouble delta_e;
    gdouble incr = 10;
    gdouble main_tick_length = 10;

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    xc = width / 2;
    yc = height / 2;

    arrow_h = height * 0.4;
    delta_e = priv->delta_e;
    arrow_w = ABS (delta_e);
    arrow_tip_w = arrow_w + 20;
    arrow_tip_h = 30;

    /*cairo_rectangle (cr, 0 , 0, width, */
    /*height); */
    /*cairo_set_source_rgba (cr, 1, 0, 0, 0.3); */
    /*cairo_fill (cr); */

    if (delta_e > 0) {
        /* arrow points upwards */
        cairo_move_to (cr, xc - arrow_w / 2, yc + arrow_h / 2);
        cairo_rel_line_to (cr, 0, -arrow_h);
        cairo_rel_line_to (cr, -(arrow_tip_w - arrow_w) / 2, 0);
        cairo_rel_line_to (cr, arrow_tip_w / 2, -arrow_tip_h);
        cairo_rel_line_to (cr, arrow_tip_w / 2, arrow_tip_h);
        cairo_rel_line_to (cr, -(arrow_tip_w - arrow_w) / 2, 0);
        cairo_rel_line_to (cr, 0, arrow_h);
    }
    else if (delta_e < 0) {
        /* arrow points downwards */
        cairo_move_to (cr, xc - arrow_w / 2, yc + arrow_h / 2);
        cairo_rel_line_to (cr, 0, -arrow_h);
        cairo_rel_line_to (cr, arrow_w, 0);
        cairo_rel_line_to (cr, 0, arrow_h);
        cairo_rel_line_to (cr, (arrow_tip_w - arrow_w) / 2, 0);
        cairo_rel_line_to (cr, -arrow_tip_w / 2, arrow_tip_h);
        cairo_rel_line_to (cr, -arrow_tip_w / 2, -arrow_tip_h);
        cairo_rel_line_to (cr, (arrow_tip_w - arrow_w) / 2, 0);
    }

    if (delta_e != 0) {
        cairo_set_source_rgba (cr, 0, 0, 0, 0.6);
        cairo_close_path (cr);
        cairo_fill_preserve (cr);
        cairo_set_source_rgba (cr, 0, 0, 0, 1);
        cairo_stroke (cr);
    }

    return TRUE;
}

static GtkSizeRequestMode
my_intensity_box_get_request_mode (GtkWidget * widget)
{
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
my_intensity_box_get_preferred_width (GtkWidget * widget,
                                      gint * minimum_size, gint * natural_size)
{
    *minimum_size = 20;
    *natural_size = 40;
}

static void
my_intensity_box_get_preferred_height (GtkWidget * widget,
                                       gint * minimum_size, gint * natural_size)
{

    /**minimum_size = 1;*/

    /**natural_size = 1;*/
}

static void
my_intensity_box_get_preferred_width_for_height (GtkWidget * widget,
                                                 gint height,
                                                 gint * minimum_width,
                                                 gint * natural_width)
{
    *minimum_width = 100;
    *natural_width = 100;
}

static void
my_intensity_box_get_preferred_height_for_width (GtkWidget * widget,
                                                 gint width,
                                                 gint * minimum_height,
                                                 gint * natural_height)
{
    *minimum_height = 100;
    *natural_height = 150;
}

static void
my_intensity_box_realize (GtkWidget * widget)
{

    MyIntensityBox *self = MY_INTENSITY_BOX (widget);
    MyIntensityBoxPrivate *priv;

    priv = my_intensity_box_get_instance_private (self);

    GTK_WIDGET_CLASS (my_intensity_box_parent_class)->realize (widget);

    gint width, height;

    GdkRectangle rect;

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    /*rect.x = 0; */
    /*rect.y = height - (priv->y / (priv->y_max - priv->y_min)) * height; */
    /*rect.height = 0; */
    /*rect.width = width; */

    /*priv->popover = gtk_popover_new (GTK_WIDGET (self)); */
    /*gtk_popover_set_modal (GTK_POPOVER (priv->popover), FALSE); */
    /*gtk_popover_set_pointing_to (GTK_POPOVER (priv->popover), &rect); */
    /*gtk_popover_set_position (GTK_POPOVER (priv->popover), GTK_POS_TOP); */
    /*gtk_container_add (GTK_CONTAINER (priv->popover), label); */
    /*gtk_container_set_border_width (GTK_CONTAINER (priv->popover), 6); */
    /*gtk_widget_show (label); */
    /*gtk_widget_set_visible (GTK_WIDGET (priv->popover), TRUE); */
}

static void
my_intensity_box_delta_energy_changed (MyIntensityBox * self,
                                       GParamSpec * pspec, gpointer data)
{
    gtk_widget_queue_draw(GTK_WIDGET(self));
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
    gobject_class->get_property = my_intensity_box_get_property;
    gobject_class->set_property = my_intensity_box_set_property;

    widget_class->draw = my_intensity_box_draw;

    widget_class->get_request_mode = my_intensity_box_get_request_mode;
    widget_class->get_preferred_width = my_intensity_box_get_preferred_width;
    widget_class->get_preferred_height = my_intensity_box_get_preferred_height;
    widget_class->get_preferred_width_for_height =
        my_intensity_box_get_preferred_width_for_height;
    widget_class->get_preferred_height_for_width =
        my_intensity_box_get_preferred_height_for_width;
    widget_class->realize = my_intensity_box_realize;

    obj_properties[PROP_DELTA_ENERGY] =
        g_param_spec_double ("delta-energy",
                           "id",
                           "unique identifier of system",
                           -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);
}

static void
my_intensity_box_init (MyIntensityBox * self)
{
    MyIntensityBoxPrivate *priv;

    priv = my_intensity_box_get_instance_private (self);

    /* to init any of the private data, do e.g: */

    g_signal_connect (self, "notify::delta-energy",
                      G_CALLBACK (my_intensity_box_delta_energy_changed), NULL);

    gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
    gtk_widget_set_halign (GTK_WIDGET (self), GTK_ALIGN_FILL);

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
