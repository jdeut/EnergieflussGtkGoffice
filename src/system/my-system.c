#include "my-system.h"

/* enum for liststore */
enum
{
    COLUMN_ARROW,
    COLUMN_ANCHOR_SOURCE,
    COLUMN_ANCHOR_SINK,
    COLUMN_ENERGY_QUANTITY,
    COLUMN_ENERGY_SINK,
    COLUMN_FROM_ENVIRONMENT,
    COLUMN_LABEL_TEXT,
    N_COLUMNS
};

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

void
my_system_draw_energy_flow (GocItem const *item, cairo_t * cr)
{
    MySystem *self;
    GocGroup *group_arrows = NULL;
    MyCanvas *canvas;
    GtkAllocation allocation;
    GtkTreeIter iter;
    gboolean valid;

    self = MY_SYSTEM (item);

    MySystemClass *class = MY_SYSTEM_GET_CLASS (self);
    GocItemClass *parent_class = g_type_class_peek_parent (class);

    /* chaining up */
    parent_class->draw (item, cr);

    g_object_get (self, "canvas", &canvas, NULL);

    group_arrows = canvas->group_arrows;

    if (!GOC_IS_GROUP (group_arrows)) {
        g_print ("can't get canvas...\n");
        return;
    }

    gtk_widget_get_allocation (GOC_WIDGET (self)->ofbox, &allocation);

    valid =
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->EnergyFlow),
                                       &iter);

    /* iterate through all arrows associated with self */
    while (valid) {

        gdouble x0, x1, y0, y1;
        GocItem *arrow;
        MySystem *sink;
        gboolean from_environment;
        gchar *label_text;
        gint anchor_source, anchor_sink;
        gfloat energy_quantity;

        // Make sure you terminate calls to gtk_tree_model_get() with a “-1” value
        gtk_tree_model_get (GTK_TREE_MODEL (self->EnergyFlow), &iter,
                            COLUMN_ANCHOR_SOURCE, &anchor_source,
                            COLUMN_ANCHOR_SINK, &anchor_sink,
                            COLUMN_ENERGY_SINK, &sink,
                            COLUMN_LABEL_TEXT, &label_text,
                            COLUMN_ENERGY_QUANTITY, &energy_quantity,
                            COLUMN_FROM_ENVIRONMENT, &from_environment,
                            COLUMN_ARROW, &arrow, -1);

        // If arrow is not instantiated yet
        if (!MY_IS_FLOW_ARROW (arrow)) {

            arrow =
                goc_item_new (group_arrows, MY_TYPE_FLOW_ARROW,
                              "energy-quantity", energy_quantity,
                              "label-text", label_text,
                              "linked-system", self, NULL);

            gtk_list_store_set (self->EnergyFlow, &iter, COLUMN_ARROW, arrow,
                                -1);
        }

        /*if(my_flow_arrow_is_dragged(MY_FLOW_ARROW(arrow))) { */
        /*continue; */
        /*} */

        /* draw arrow */

        x0 = allocation.x + allocation.width / 2;
        y0 = allocation.y + allocation.height / 2;

        gdouble length = allocation.width * 0.66;

        /* if arrow depicts transfer to other system */
        if (MY_IS_SYSTEM (sink)) {

            gdouble dx, dy, alpha;
            GtkAllocation alloc_sink;

            gtk_widget_get_allocation (GOC_WIDGET (sink)->ofbox, &alloc_sink);

            dx = alloc_sink.x - allocation.x;
            dy = alloc_sink.y - allocation.y;

            alpha = atan2 (dy, dx);

            anchor_sink = ANCHOR_SOUTH;

            if (-M_PI / 4 < alpha && alpha <= M_PI / 4) {
                anchor_sink = ANCHOR_WEST;
            }
            else if (M_PI / 4 < alpha && alpha <= 3 * M_PI / 4) {
                anchor_sink = ANCHOR_NORTH;
            }
            else if (3 * M_PI / 4 < alpha || alpha <= -3 * M_PI / 4) {
                anchor_sink = ANCHOR_EAST;
            }

            if (anchor_sink == ANCHOR_WEST) {
                x1 = alloc_sink.x;
                y1 = alloc_sink.y + alloc_sink.height / 2;
            }
            else if (anchor_sink == ANCHOR_SOUTH) {
                x1 = alloc_sink.x + alloc_sink.width / 2;
                y1 = alloc_sink.y + alloc_sink.height;
            }
            else if (anchor_sink == ANCHOR_NORTH) {
                x1 = alloc_sink.x + alloc_sink.width / 2;
                y1 = alloc_sink.y;
            }
            else if (anchor_sink == ANCHOR_EAST) {
                x1 = alloc_sink.x + alloc_sink.width;
                y1 = alloc_sink.y + alloc_sink.height / 2;
            }

        }
        /* if arrow depicts transfer to environment */
        else if (!from_environment) {

            if (anchor_source == ANCHOR_WEST) {
                x0 = allocation.x + allocation.width;
                x1 = x0 + length;
                y1 = y0;
            }
            else if (anchor_source == ANCHOR_EAST) {
                x0 = allocation.x;
                x1 = allocation.x - length;
                y1 = y0;
            }
            else if (anchor_source == ANCHOR_SOUTH) {
                x1 = x0;
                y0 = allocation.y + allocation.height;
                y1 = allocation.y + allocation.height + length;
            }
            else if (anchor_source == ANCHOR_NORTH) {
                x1 = x0;
                y0 = allocation.y;
                y1 = allocation.y - length;
            }
        }
        /* if arrow depicts transfer from environment */
        else if (from_environment) {

            if (anchor_source == ANCHOR_WEST) {
                x1 = allocation.x + allocation.width;
                x0 = x1 + length;
                y1 = y0;
            }
            else if (anchor_source == ANCHOR_EAST) {
                x1 = allocation.x;
                x0 = x1 - length;
                y1 = y0;
            }
            else if (anchor_source == ANCHOR_SOUTH) {
                x1 = x0;
                y1 = allocation.y + allocation.height;
                y0 = y1 + length;
            }
            else if (anchor_source == ANCHOR_NORTH) {
                y1 = allocation.y;
                y0 = y1 - length;
                x1 = x0;
            }
        }

        if (!my_flow_arrow_is_dragged (MY_FLOW_ARROW (arrow))) {
            goc_item_set (arrow, "x0", x0, "y0", y0, "x1", x1, "y1", y1, NULL);
        }

        valid =
            gtk_tree_model_iter_next (GTK_TREE_MODEL (self->EnergyFlow), &iter);

    }
}

static void
notify_canvas_changed_cb (MySystem * self, GParamSpec * pspec, gpointer data)
{
    g_print ("canvas changed\n");
}

static gboolean
my_system_button_pressed (GocItem * item, int button, double x, double y)
{
    g_print ("MySystem button pressed...\n");
}

static void
my_system_class_init (MySystemClass * klass)
{
    GObjectClass *gobject_class;
    GocItemClass *gi_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gi_class = (GocItemClass *) klass;

    /*g_type_class_add_private (gobject_class, sizeof (MySystemPrivate)); */

    gobject_class->finalize = my_system_finalize;
    gobject_class->dispose = my_system_dispose;

    gi_class->button_pressed = my_system_button_pressed;
    gi_class->draw = my_system_draw_energy_flow;
}

void
my_system_change_flow_arrow_direction (MySystem * self, MyFlowArrow * arrow)
{
    GtkWidget *canvas;
    gboolean valid;
    GtkTreeIter iter;

    g_return_val_if_fail (MY_IS_SYSTEM (self), FALSE);
    g_return_val_if_fail (MY_IS_FLOW_ARROW (arrow), FALSE);

    valid =
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->EnergyFlow),
                                       &iter);

    /* iterate through all arrows associated with self */
    while (valid) {

        MyFlowArrow *iter_arrow;
        MySystem *iter_sink;
        gfloat iter_energy_quantity;
        gchar *iter_label;
        gint iter_from_environment, iter_anchor_sink;


        gtk_tree_model_get (GTK_TREE_MODEL (self->EnergyFlow), &iter,
                            COLUMN_ENERGY_QUANTITY, &iter_energy_quantity,
                            COLUMN_ANCHOR_SINK, &iter_anchor_sink,
                            COLUMN_ENERGY_SINK, &iter_sink,
                            COLUMN_LABEL_TEXT, &iter_label,
                            COLUMN_FROM_ENVIRONMENT, &iter_from_environment,
                            COLUMN_ARROW, &iter_arrow, -1);

        if (iter_arrow == arrow) {

            /* if arrow connects to systems */
            if (MY_IS_SYSTEM (iter_sink)) {
                my_system_add_energy_transfer_to_system (iter_sink, iter_label,
                                                         iter_anchor_sink,
                                                         iter_energy_quantity,
                                                         self);
                my_system_remove_flow_arrow (self, arrow);
            }
            else {

                if (iter_from_environment)
                    iter_from_environment = FALSE;
                else
                    iter_from_environment = TRUE;

                gtk_list_store_set (self->EnergyFlow, &iter,
                                    COLUMN_FROM_ENVIRONMENT,
                                    iter_from_environment, -1);
            }

            break;
        }

        valid =
            gtk_tree_model_iter_next (GTK_TREE_MODEL (self->EnergyFlow), &iter);
    }

    goc_item_invalidate (GOC_ITEM (self));
    goc_item_invalidate (GOC_ITEM (arrow));
}

gboolean
my_system_remove_flow_arrow (MySystem * self, MyFlowArrow * arrow)
{
    gboolean valid;
    GtkTreeIter iter;

    g_return_val_if_fail (MY_IS_SYSTEM (self), FALSE);
    g_return_val_if_fail (MY_IS_FLOW_ARROW (arrow), FALSE);

    valid =
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->EnergyFlow),
                                       &iter);

    /* iterate through all arrows associated with self */
    while (valid) {

        MyFlowArrow *iter_arrow;
        MySystem *iter_sink;
        gint iter_from_environment;


        gtk_tree_model_get (GTK_TREE_MODEL (self->EnergyFlow), &iter,
                            COLUMN_ENERGY_SINK, &iter_sink,
                            COLUMN_FROM_ENVIRONMENT, &iter_from_environment,
                            COLUMN_ARROW, &iter_arrow, -1);

        if (iter_arrow == arrow) {

            my_flow_arrow_destroy (arrow);

            gtk_list_store_remove (self->EnergyFlow, &iter);

            break;
        }

        valid =
            gtk_tree_model_iter_next (GTK_TREE_MODEL (self->EnergyFlow), &iter);
    }
}


gboolean
my_system_change_sink_of_arrow (MySystem * self, MyFlowArrow * arrow,
                                MySystem * sink)
{
    gboolean valid;
    GtkTreeIter iter;

    g_return_val_if_fail (MY_IS_SYSTEM (self), FALSE);
    g_return_val_if_fail (MY_IS_SYSTEM (sink) || sink == NULL, FALSE);
    g_return_val_if_fail (MY_IS_FLOW_ARROW (arrow), FALSE);

    valid =
        gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->EnergyFlow),
                                       &iter);

    /* iterate through all arrows associated with self */
    while (valid) {

        MyFlowArrow *iter_arrow;

        gtk_tree_model_get (GTK_TREE_MODEL (self->EnergyFlow), &iter,
                            COLUMN_ARROW, &iter_arrow, -1);

        if(iter_arrow == arrow) {
            g_print("laksfn\n");

            gtk_list_store_set (self->EnergyFlow, &iter,
                                COLUMN_ENERGY_SINK, sink, -1);
        }

        valid =
            gtk_tree_model_iter_next (GTK_TREE_MODEL (self->EnergyFlow), &iter);
    }

    return TRUE;
}

gboolean
my_system_add_energy_transfer_to_system (MySystem * self, gchar * label_text,
                                         gint anchor_sink, gfloat quantity,
                                         MySystem * sink)
{
    GtkTreeIter iter;

    g_return_val_if_fail (MY_IS_SYSTEM (self), FALSE);
    g_return_val_if_fail (MY_IS_SYSTEM (sink), FALSE);

    gtk_list_store_append (self->EnergyFlow, &iter);

    gtk_list_store_set (self->EnergyFlow, &iter,
                        COLUMN_ANCHOR_SINK, anchor_sink,
                        COLUMN_ENERGY_QUANTITY, quantity,
                        COLUMN_LABEL_TEXT, label_text,
                        COLUMN_FROM_ENVIRONMENT, FALSE,
                        COLUMN_ENERGY_SINK, sink, -1);
}

gboolean
my_system_add_energy_transfer_to_environment (MySystem * self,
                                              gchar * label_text,
                                              gint anchor_source,
                                              gfloat quantity)
{
    GtkTreeIter iter;

    g_return_val_if_fail (MY_IS_SYSTEM (self), FALSE);

    gtk_list_store_append (self->EnergyFlow, &iter);

    gtk_list_store_set (self->EnergyFlow, &iter,
                        COLUMN_ANCHOR_SOURCE, anchor_source,
                        COLUMN_ENERGY_QUANTITY, quantity,
                        COLUMN_LABEL_TEXT, label_text,
                        COLUMN_FROM_ENVIRONMENT, FALSE, -1);
}

gboolean
my_system_add_energy_transfer_from_environment (MySystem * self,
                                                gchar * label_text,
                                                gint anchor_source,
                                                gfloat quantity)
{
    GtkTreeIter iter;

    g_return_val_if_fail (MY_IS_SYSTEM (self), FALSE);

    gtk_list_store_append (self->EnergyFlow, &iter);

    gtk_list_store_set (self->EnergyFlow, &iter,
                        COLUMN_ANCHOR_SOURCE, anchor_source,
                        COLUMN_ENERGY_QUANTITY, quantity,
                        COLUMN_LABEL_TEXT, label_text,
                        COLUMN_FROM_ENVIRONMENT, TRUE, -1);
}

static void
my_system_init_energy_flow_store (MySystem * self)
{
    self->EnergyFlow =
        gtk_list_store_new (N_COLUMNS,
                            GOC_TYPE_LINE,
                            G_TYPE_INT, G_TYPE_INT, G_TYPE_FLOAT,
                            MY_TYPE_SYSTEM, G_TYPE_BOOLEAN, G_TYPE_STRING);
}

static void
my_system_init (MySystem * self)
{
    GtkWidget *button;

    button = gtk_button_new_with_label ("MyNewSystem");

    goc_item_set (GOC_ITEM (self), "widget", button, "width", 150.0, "height",
                  80.0, NULL);

    my_system_init_energy_flow_store (self);

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
