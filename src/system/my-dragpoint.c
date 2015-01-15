#include "my-dragpoint.h"

/* 'private'/'protected' functions */
static void my_drag_point_class_init (MyDragPointClass * klass);
static void my_drag_point_init (MyDragPoint * self);
static void my_drag_point_finalize (GObject * );
static void my_drag_point_dispose (GObject * );

struct _MyDragPointPrivate
{
    /* private members go here */
};

#define MY_DRAG_POINT_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                       MY_TYPE_DRAG_POINT, \
                                       MyDragPointPrivate))

G_DEFINE_TYPE (MyDragPoint, my_drag_point, GOC_TYPE_CIRCLE);


GQuark
my_drag_point_error_quark (void)
{
  return g_quark_from_static_string ("my-drag-point-error-quark");
}

gboolean
my_drag_point_button_pressed (GocItem * item, int button, double x, double y)
{
    GOStyle *style;
    cairo_surface_t *surf;
    cairo_t *cr;
    GOStyledObject *gso = GO_STYLED_OBJECT(item);
    MyDragPoint *self = MY_DRAG_POINT (item);
    MyDragPointClass *class = MY_DRAG_POINT_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    parent_class->button_pressed (item, button, x, y);

    g_print ("button pressed...\n");

    style = go_style_dup (go_styled_object_get_style (gso));
    style->line.width = 2;
    style->fill.type = GO_STYLE_FILL_PATTERN;
    style->fill.pattern.pattern = GO_PATTERN_SOLID;
    style->fill.pattern.back = GO_COLOR_RED;
    style->fill.pattern.fore = GO_COLOR_RED;
    go_styled_object_set_style (gso, style);

    g_object_unref (style);

    return FALSE;
}

static void
my_drag_point_class_init (MyDragPointClass * klass)
{
    GObjectClass *gobject_class;
    GocItemClass *gi_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gi_class = (GocItemClass *) klass;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_drag_point_finalize;
    gobject_class->dispose = my_drag_point_dispose;

    g_type_class_add_private (gobject_class,
                              sizeof (MyDragPointPrivate));

    gi_class->button_pressed = my_drag_point_button_pressed;
}

static void
my_drag_point_init (MyDragPoint * self)
{
    self->_priv = MY_DRAG_POINT_GET_PRIVATE (self);

    /* to init any of the private data, do e.g: */

    /* 	obj->_priv->_frobnicate_mode = FALSE; */
}

static void
my_drag_point_dispose (GObject *object)
{
    G_OBJECT_CLASS (my_drag_point_parent_class)->dispose (object);
}

static void
my_drag_point_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_drag_point_parent_class)->finalize (object);
}

MyDragPoint *
my_drag_point_new (void)
{
    MyDragPoint *self;

    self = g_object_new (MY_TYPE_DRAG_POINT, NULL);

    return self;
}
