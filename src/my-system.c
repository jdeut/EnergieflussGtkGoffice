#include "my-system.h"

/******************************************************************************
 * GocOffscreenBox: code mostly copied from gtk+/tests/gtkoffscreenbox.[c,h]
 ******************************************************************************/

#define GOC_TYPE_OFFSCREEN_BOX              (goc_offscreen_box_get_type ())
#define GOC_OFFSCREEN_BOX(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOC_TYPE_OFFSCREEN_BOX, GocOffscreenBox))
#define GOC_OFFSCREEN_BOX_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GOC_TYPE_OFFSCREEN_BOX, GocOffscreenBoxClass))
#define GOC_IS_OFFSCREEN_BOX(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GOC_TYPE_OFFSCREEN_BOX))
#define GOC_IS_OFFSCREEN_BOX_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GOC_TYPE_OFFSCREEN_BOX))
#define GOC_OFFSCREEN_BOX_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GOC_TYPE_OFFSCREEN_BOX, GocOffscreenBoxClass))

typedef struct _GocOffscreenBox GocOffscreenBox;
typedef GtkBinClass GocOffscreenBoxClass;

static GObjectClass *goc_offscreen_box_parent_class;

struct _GocOffscreenBox
{
    GtkBin base;
    GtkWidget *child;
    GdkWindow *offscreen_window;
    gdouble angle, scale;
};

struct _GocOffscreenBoxClass
{
    GtkBinClass parent_class;
};

/******************************************************************************
 * GocOffscreenBox: End
 ******************************************************************************/

/* 'private'/'protected' functions */
static void my_system_class_init (MySystemClass * klass);
static void my_system_init (MySystem * self);
static void my_system_finalize (GObject *);
static void my_system_dispose (GObject *);

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
ofbox_size_allocate_cb (GtkWidget * widget, GdkRectangle * allocation,
                        MySystem * self)
{
    if (self->line_out == NULL)
        return;

    gdouble x0, x1, y0, y1;

    x0 = allocation->x + allocation->width;
    x1 = allocation->x + allocation->width + 50;
    y0 = allocation->y + allocation->height / 2;
    y1 = allocation->y + allocation->height / 2;

    goc_item_set (self->line_out, "x0", x0, "y0", y0, "x1", x1, "y1", y1, NULL);
}

gboolean
widget_draw_cb (GtkWidget * widget, cairo_t * cr, gpointer user_data)
{
    /*g_print("draw...\n"); */

    return FALSE;
}

static void
notify_widget_changed_cb (MySystem * self, GParamSpec * pspec, gpointer data)
{
    GocWidget *goc_widget = GOC_WIDGET (self);
    GocOffscreenBox *ofbox = GOC_OFFSCREEN_BOX (goc_widget->ofbox);

    g_print ("'widget' property changed...\n");

    g_signal_connect (ofbox, "size-allocate",
                      G_CALLBACK (ofbox_size_allocate_cb), self);

    g_signal_connect (goc_widget->widget, "draw", G_CALLBACK (widget_draw_cb),
                      self);
}

static void
notify_canvas_changed_cb (MySystem * self, GParamSpec * pspec, gpointer data)
{
    GocGroup *top_level_group = NULL;
    GOArrow *arr;
    GOStyle *style;
    GocWidget *goc_widget = GOC_WIDGET (self);

    g_print ("canvas changed\n");

    top_level_group = goc_canvas_get_root (goc_widget->base.canvas);

    if (top_level_group == NULL) {
        g_print ("can't get canvas...\n");
        return;
    }

    arr = g_new0 (GOArrow, 1);

    go_arrow_init_kite (arr, 20, 20, 4);

    self->line_out =
        goc_item_new (top_level_group, GOC_TYPE_LINE, "x0", 0.0, "x1", 1.0,
                      "y1", 0.0, "y0", 1.0, "end-arrow", arr, NULL);

    g_object_get (G_OBJECT (self->line_out), "style", &style, NULL);

    style->line.width = 20;
    style->line.color = GO_COLOR_FROM_RGBA (0, 200, 0, 255);

    goc_item_lower_to_bottom (GOC_ITEM (self->line_out));

    if (self->line_out == NULL) {
        g_print ("err...\n");
    }
}


static void
my_system_class_init (MySystemClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = my_system_finalize;
    gobject_class->dispose = my_system_dispose;
    /*g_type_class_add_private (gobject_class, sizeof (MySystemPrivate)); */
}

static void
my_system_init (MySystem * self)
{
    GtkWidget *button;

    self->line_out = NULL;

    button = gtk_button_new_with_label ("MyNewSystem");
    goc_item_set (GOC_ITEM (self), "widget", button, "x", 250.0,
                  "y", 350.0, "width", 100.0, "height", 50.0, NULL);

    g_signal_connect (self, "notify::widget",
                      G_CALLBACK (notify_widget_changed_cb), NULL);

    g_signal_connect (self, "notify::canvas",
                      G_CALLBACK (notify_canvas_changed_cb), NULL);

}

static void
my_system_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_system_parent_class)->dispose (object);
}

static void
my_system_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_system_parent_class)->finalize (object);
}
