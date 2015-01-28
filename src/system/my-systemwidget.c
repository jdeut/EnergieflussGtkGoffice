#include "my-systemwidget.h"

/* 'private'/'protected' functions */
static void my_system_widget_class_init (MySystemWidgetClass * klass);
static void my_system_widget_init (MySystemWidget * self);
static void my_system_widget_finalize (GObject *);
static void my_system_widget_dispose (GObject *);

enum
{
    PROP_0,
    PROP_MODEL,
    PROP_ID,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

struct _MySystemWidgetPrivate
{
    /* private members go here */
    GtkImage *image;
    GtkWidget *label1;
    GtkWidget *button_properties;

    GdkPixbuf *pixbuf;

    MySystemModel *model;
    guint id;
};

G_DEFINE_TYPE_WITH_PRIVATE (MySystemWidget, my_system_widget,
                            GTK_TYPE_EVENT_BOX);

GQuark
my_system_widget_error_quark (void)
{
    return g_quark_from_static_string ("my-system-widget-error-quark");
}

static void
my_system_widget_set_property (GObject * object,
                               guint property_id,
                               const GValue * value, GParamSpec * pspec)
{
    MySystemWidget *self = MY_SYSTEM_WIDGET (object);

    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    switch (property_id) {

        case PROP_MODEL:
            priv->model = g_value_get_object (value);
            break;

        case PROP_ID:
            priv->id = g_value_get_uint (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_system_widget_get_property (GObject * object,
                               guint property_id, GValue * value,
                               GParamSpec * pspec)
{
    MySystemWidget *self = MY_SYSTEM_WIDGET (object);

    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);


    switch (property_id) {

        case PROP_MODEL:
            g_value_set_object (value, priv->model);
            break;

        case PROP_ID:
            g_value_set_uint (value, priv->id);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
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
    gobject_class->set_property = my_system_widget_set_property;
    gobject_class->get_property = my_system_widget_get_property;

    obj_properties[PROP_MODEL] =
        g_param_spec_object ("model",
                             "data model",
                             "data model",
                             MY_TYPE_SYSTEM_MODEL, G_PARAM_READWRITE);

    obj_properties[PROP_ID] =
        g_param_spec_uint ("id",
                           "id",
                           "unique identifier of system",
                           0, G_MAXUINT, 0,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/org/gtk/myapp/my-system-widget.ui");

    gtk_widget_class_bind_template_child_private (widget_class, MySystemWidget,
                                                  image);
    gtk_widget_class_bind_template_child_private (widget_class, MySystemWidget,
                                                  label1);
    gtk_widget_class_bind_template_child_private (widget_class, MySystemWidget,
                                                  button_properties);
}

gboolean
my_system_enter_event (MySystemWidget * self,
                       GdkEvent * event, gpointer user_data)
{
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_ACTIVE, TRUE);

    return FALSE;
}

gboolean
my_system_leave_event (MySystemWidget * self,
                       GdkEvent * event, gpointer user_data)
{
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_NORMAL, TRUE);

    return FALSE;
}

void
my_system_widget_model_changed (MySystemWidget * self,
                                GParamSpec * pspec, gpointer user_data)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    guint id;

    g_object_get (priv->model, "id", &id, NULL);

    gchar *str = g_strdup_printf ("id: %u", id);

    gtk_label_set_text (GTK_LABEL (priv->label1), str);
    g_print ("model changed of system\n");
}

void
my_system_widget_button_properties_clicked (MySystemWidget * self,
                                            gpointer data)
{
    GtkWidget *toplevel;

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    g_return_if_fail (MY_IS_WINDOW (toplevel));

    my_system_widget_properties_dialog_show (GTK_WINDOW (toplevel), self);
}

static void
my_system_widget_init (MySystemWidget * self)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    /* to init any of the private data, do e.g: */

    priv->model = NULL;

    gtk_widget_init_template (GTK_WIDGET (self));

    g_signal_connect (self, "enter-notify-event",
                      G_CALLBACK (my_system_enter_event), NULL);
    g_signal_connect (self, "leave-notify-event",
                      G_CALLBACK (my_system_leave_event), NULL);
    g_signal_connect (self, "notify::model",
                      G_CALLBACK (my_system_widget_model_changed), NULL);

    g_signal_connect_swapped (priv->button_properties, "clicked",
                              G_CALLBACK
                              (my_system_widget_button_properties_clicked),
                              self);
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

void
my_system_widget_timeline_current_index_changed (MySystemWidget * self,
                                                 MyTimelineModel * timeline)
{
    MySystemWidgetPrivate *priv;
    MySystemModel *model;
    GPtrArray *data;

    priv = my_system_widget_get_instance_private (self);

    data = my_timeline_model_get_systems_data_of_current_pos (timeline);

    if (priv->id <= data->len) {
        model = g_ptr_array_index (data, priv->id);
    }

    g_return_if_fail (MY_IS_SYSTEM_MODEL (model));

    g_object_set (self, "model", model, NULL);
}

void
my_system_widget_realized (MySystemWidget * self, gpointer data)
{
    MyTimelineModel *timeline;
    GtkWidget *toplevel;

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    g_return_if_fail (MY_IS_WINDOW (toplevel));

    timeline = my_window_get_timeline (MY_WINDOW (toplevel));

    g_return_if_fail (MY_IS_TIMELINE_MODEL (timeline));

    g_signal_connect_swapped (timeline, "current-pos-changed",
                              G_CALLBACK
                              (my_system_widget_timeline_current_index_changed),
                              self);
}

MySystemWidget *
my_system_widget_new (void)
{
    MySystemWidget *self;
    MySystemWidgetPrivate *priv;

    self = g_object_new (MY_TYPE_SYSTEM_WIDGET, NULL);

    priv = my_system_widget_get_instance_private (self);

    priv->pixbuf =
        gdk_pixbuf_new_from_resource_at_scale ("/org/gtk/myapp/lagerfeuer.png",
                                               200, -1, TRUE, NULL);

    g_signal_connect (self, "realize", G_CALLBACK (my_system_widget_realized),
                      NULL);

    gtk_image_set_from_pixbuf (priv->image, priv->pixbuf);

    return self;
}
