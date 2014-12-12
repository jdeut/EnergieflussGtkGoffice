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

gboolean
my_system_draw_energy_flow (MySystem * self)
{

    GocGroup *toplevel = NULL;
    GtkAllocation allocation;
    GtkTreeIter iter;
    gboolean valid;

    toplevel = goc_canvas_get_root (GOC_WIDGET (self)->base.canvas);

    if (toplevel == NULL) {
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
        GocItem *line;
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
                            COLUMN_ARROW, &line, -1);

        // If line is not instantiated yet
        if (!MY_IS_FLOW_ARROW (line)) {

            line =
                goc_item_new (toplevel, MY_TYPE_FLOW_ARROW,
                              "energy-quantity", energy_quantity,
                              "label-text", label_text,
                              "linked-system", self, NULL);

            gtk_list_store_set (self->EnergyFlow, &iter, COLUMN_ARROW, line,
                                -1);
        }

        /* draw line */

        x0 = allocation.x + allocation.width / 2;
        y0 = allocation.y + allocation.height / 2;

        gdouble length = allocation.width * 0.66;

        /* if arrow depicts transfer to other system */
        if (MY_IS_SYSTEM (sink)) {

            GtkAllocation alloc_sink;

            gtk_widget_get_allocation (GOC_WIDGET (sink)->ofbox, &alloc_sink);

            if (anchor_sink == ANCHOR_WEST) {
                x1 = alloc_sink.x + alloc_sink.width;
                y1 = alloc_sink.y + alloc_sink.height / 2;
            }
            else if (anchor_sink == ANCHOR_SOUTH) {
                x1 = alloc_sink.x + alloc_sink.width / 2;
                y1 = alloc_sink.y + alloc_sink.height;
            }
            else if (anchor_sink == ANCHOR_EAST) {
                x1 = alloc_sink.x;
                y1 = alloc_sink.y + alloc_sink.height / 2;
            }

        }
        /* if arrow depicts transfer to environment */
        else if (!from_environment) {

            if (anchor_source == ANCHOR_WEST) {
                x1 = allocation.x + allocation.width + length;
                y1 = y0;
            }
            else if (anchor_source == ANCHOR_EAST) {
                x1 = allocation.x - length;
                y1 = y0;
            }
            else if (anchor_source == ANCHOR_SOUTH) {
                x1 = x0;
                y1 = allocation.y + allocation.height + length;
            }
            else if (anchor_source == ANCHOR_NORTH) {
                x1 = x0;
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

        goc_item_set (line, "x0", x0, "y0", y0, "x1", x1, "y1", y1, NULL);

        valid =
            gtk_tree_model_iter_next (GTK_TREE_MODEL (self->EnergyFlow), &iter);

    }
}

static void
ofbox_size_allocate_cb (GtkWidget * widget, GdkRectangle * allocation,
                        MySystem * self)
{
    GSList *tmp;

    tmp = self->AssociatedSystems;

    while (tmp) {
        MySystem *associate = tmp->data;

        if (MY_IS_SYSTEM (associate)) {
            /* draw all energy flows of those systems that transfer
             * energy to self */
            my_system_draw_energy_flow (associate);
        }

        tmp = tmp->next;
    }

    my_system_draw_energy_flow (self);
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
    GocOffscreenBox *ofbox = GOC_OFFSCREEN_BOX (GOC_WIDGET (self)->ofbox);

    g_print ("'widget' property changed...\n");

    g_signal_connect (ofbox, "size-allocate",
                      G_CALLBACK (ofbox_size_allocate_cb), self);

    /*g_signal_connect (goc_widget->widget, "draw", G_CALLBACK (widget_draw_cb), */
    /*self); */
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
}

void
my_system_remove_associate (MySystem * self, MySystem * associate)
{
    g_return_if_fail (MY_IS_SYSTEM (self));
    g_return_if_fail (MY_IS_SYSTEM (associate));

    self->AssociatedSystems =
        g_slist_remove (self->AssociatedSystems, associate);
}

void
my_system_add_associate (MySystem * self, MySystem * associate)
{
    g_return_if_fail (MY_IS_SYSTEM (self));
    g_return_if_fail (MY_IS_SYSTEM (associate));

    self->AssociatedSystems =
        g_slist_append (self->AssociatedSystems, associate);
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

            if(MY_IS_SYSTEM(iter_sink)) {
                my_system_remove_associate(self, iter_sink);
            }

            my_flow_arrow_destroy (arrow);

            gtk_list_store_remove (self->EnergyFlow, &iter);

            break;
        }

        valid =
            gtk_tree_model_iter_next (GTK_TREE_MODEL (self->EnergyFlow), &iter);
    }
}

gboolean
my_system_add_energy_transfer_to_system (MySystem * self, gchar * label_text,
                                         gint anchor_sink, gfloat quantity,
                                         MySystem * sink)
{
    GtkTreeIter iter;

    g_return_val_if_fail (MY_IS_SYSTEM (self), FALSE);
    g_return_val_if_fail (MY_IS_SYSTEM (sink), FALSE);

    /* associate sink with self, so that energy_flows of self are
     * redrawn if the allocation of sink changes */

    my_system_add_associate (sink, self);

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
my_system_init_associated_systems (MySystem * self)
{
    self->AssociatedSystems = NULL;
}

static void
my_system_init_energy_flow_store (MySystem * self)
{
    self->EnergyFlow =
        gtk_list_store_new (N_COLUMNS,
                            GOC_TYPE_LINE,
                            G_TYPE_INT,
                            G_TYPE_INT,
                            G_TYPE_FLOAT,
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
    my_system_init_associated_systems (self);

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
