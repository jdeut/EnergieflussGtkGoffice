#include "my-systemwidget.h"

/* 'private'/'protected' functions */
static void my_system_widget_class_init (MySystemWidgetClass * klass);
static void my_system_widget_init (MySystemWidget * self);
static void my_system_widget_finalize (GObject *);
static void my_system_widget_dispose (GObject *);
void my_system_widget_update (MySystemWidget * self);
void my_system_widget_initialize_handlers (MySystemWidget *, MySystemModel *);
void my_system_widget_specific_model_added (MySystemWidget * self,
                                            GParamSpec * pspec,
                                            gpointer user_data);
void model_handler_picture_path_changed (MySystemWidget * self,
                                         GParamSpec * pspec,
                                         MySystemModel * model);
void my_system_widget_set_label_from_model (MySystemWidget * self,
                                            MySystemModel * model);
void my_system_widget_realized (MySystemWidget * self, gpointer data);
void my_system_widget_set_pixbuf_from_model (MySystemWidget * self,
                                             MySystemModel * model);

enum
{
    PROP_0,
    PROP_SPECIFIC_MODEL,
    PROP_GENERIC_MODEL,
    PROP_ID,
    PROP_SYSTEM,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

enum
{
    MODEL_PICTURE_PATH_CHANGED,
    MODEL_LABEL_CHANGED,
    N_MODEL_HANDLER
};

enum
{
    MODEL_SPECIFIC,
    MODEL_GENERIC,
    N_MODEL
};

struct _MySystemWidgetPrivate
{
    /* private members go here */
    GtkImage *image;
    GtkWidget *label1;
    GtkWidget *button_properties;
    GtkWidget *stack;

    MyIntensityBox *ib;

    GdkPixbuf *pixbuf_orig;
    GdkPixbuf *pixbuf_scaled;

    MySystemModel *model[N_MODEL];

    MySystem *system;

    gulong model_handler[N_MODEL][N_MODEL_HANDLER];

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

        case PROP_SPECIFIC_MODEL:
            priv->model[MODEL_SPECIFIC] = g_value_get_object (value);
            break;

        case PROP_GENERIC_MODEL:
            priv->model[MODEL_GENERIC] = g_value_get_object (value);
            break;

        case PROP_SYSTEM:
            priv->system = g_value_get_object (value);
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

        case PROP_SPECIFIC_MODEL:
            g_value_set_object (value, priv->model[MODEL_SPECIFIC]);
            break;

        case PROP_GENERIC_MODEL:
            g_value_set_object (value, priv->model[MODEL_GENERIC]);
            break;

        case PROP_SYSTEM:
            g_value_set_object (value, priv->system);
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

    obj_properties[PROP_SPECIFIC_MODEL] =
        g_param_spec_object ("specific-model",
                             "specific data model",
                             "data model",
                             MY_TYPE_SYSTEM_MODEL, G_PARAM_READWRITE);

    obj_properties[PROP_GENERIC_MODEL] =
        g_param_spec_object ("generic-model",
                             "generic data model",
                             "data model",
                             MY_TYPE_SYSTEM_MODEL, G_PARAM_READWRITE);

    obj_properties[PROP_SYSTEM] =
        g_param_spec_object ("system",
                             "parent system",
                             "parent system",
                             MY_TYPE_SYSTEM, G_PARAM_READWRITE);

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
                                                  stack);
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

static void
my_system_widget_pixbuf_set_proper_size (MySystemWidget * self)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    cairo_rectangle_t rect;
    gint p_w, p_h, dest_w, dest_h;

    if (!MY_IS_SYSTEM (priv->system)) {
        return;
    }

    g_object_get (priv->system, "width", &rect.width, "height", &rect.height,
                  NULL);

    rect.width = ((gfloat) 0.8 * rect.width);
    rect.height = ((gfloat) 0.7 * rect.height);

    g_return_if_fail (GDK_IS_PIXBUF (priv->pixbuf_orig));

    p_w = gdk_pixbuf_get_width (priv->pixbuf_orig);
    p_h = gdk_pixbuf_get_height (priv->pixbuf_orig);

    if (((gfloat) rect.width / rect.height) >= ((gfloat) p_w / p_h)) {
        dest_h = rect.height;
        dest_w = p_w * rect.height / p_h;
    }
    else {
        dest_h = p_h * rect.width / p_w;
        dest_w = rect.width;
    }

    if (GDK_IS_PIXBUF (priv->pixbuf_scaled))
        g_object_unref (priv->pixbuf_scaled);

    priv->pixbuf_scaled =
        gdk_pixbuf_scale_simple (priv->pixbuf_orig, dest_w, dest_h,
                                 GDK_INTERP_BILINEAR);
}

void
model_handler_label_changed (MySystemWidget * self, GParamSpec * pspec,
                             MySystemModel * model)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    my_system_widget_update (self);
}

void
model_handler_picture_path_changed (MySystemWidget * self, GParamSpec * pspec,
                                    MySystemModel * model)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    gchar *picture_path;
    GError *err = NULL;
    GdkPixbuf *pixbuf_new, *pixbuf_tmp;

    g_return_if_fail (MY_IS_SYSTEM_MODEL (model));
    g_return_if_fail (MY_IS_SYSTEM_WIDGET (self));

    g_object_get (model, "picture-path", &picture_path, NULL);

    if (picture_path == NULL)
        return;

    pixbuf_new = gdk_pixbuf_new_from_file (picture_path, &err);

    if (err) {

        GtkWidget *toplevel;
        gchar *str;

        toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

        str =
            g_strdup_printf ("Failed to load file '%s' into pixbuf",
                             picture_path);

        my_window_caution (toplevel, str);
        g_free (str);
        g_error_free (err);

        return;
    }

    pixbuf_tmp = priv->pixbuf_orig;

    priv->pixbuf_orig = pixbuf_new;

    g_object_unref (pixbuf_tmp);

    my_system_widget_pixbuf_set_proper_size (self);

    g_object_set (model, "pixbuf", priv->pixbuf_orig, NULL);

    my_system_widget_update (self);
}

void
my_system_widget_set_label_from_model (MySystemWidget * self,
                                       MySystemModel * model)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);
    gchar *label;

    g_object_get (model, "label", &label, NULL);

    gtk_label_set_text (GTK_LABEL (priv->label1), label);

    g_free (label);
}

void
my_system_widget_set_pixbuf_from_model (MySystemWidget * self,
                                        MySystemModel * model)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);
    GdkPixbuf *pixbuf;

    g_object_get (model, "pixbuf", &pixbuf, NULL);

    if (pixbuf != NULL) {
        priv->pixbuf_orig = pixbuf;
    }
    else {
        priv->pixbuf_orig =
            gdk_pixbuf_new_from_resource_at_scale
            ("/org/gtk/myapp/lagerfeuer.png", 200, -1, TRUE, NULL);
    }

    my_system_widget_pixbuf_set_proper_size (self);

    gtk_image_set_from_pixbuf (priv->image, priv->pixbuf_scaled);
}

void
my_system_widget_update (MySystemWidget * self)
{

    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    GdkPixbuf *pixbuf;
    gchar *label;

    g_object_get (priv->model[MODEL_SPECIFIC], "pixbuf", &pixbuf, NULL);

    if (pixbuf == NULL)
        my_system_widget_set_pixbuf_from_model (self,
                                                priv->model[MODEL_GENERIC]);
    else {
        g_object_unref (pixbuf);
        my_system_widget_set_pixbuf_from_model (self,
                                                priv->model[MODEL_SPECIFIC]);
    }

    g_object_get (priv->model[MODEL_SPECIFIC], "label", &label, NULL);

    if (label == NULL)
        my_system_widget_set_label_from_model (self,
                                               priv->model[MODEL_GENERIC]);
    else
        my_system_widget_set_label_from_model (self,
                                               priv->model[MODEL_SPECIFIC]);

    g_free (label);
}

void
my_system_widget_specific_model_added (MySystemWidget * self,
                                       GParamSpec * pspec, gpointer user_data)
{
    my_system_widget_update (self);
}

void
my_system_widget_button_properties_clicked (MySystemWidget * self,
                                            gpointer data)
{
    GtkWidget *toplevel;
    SystemSettings ss;

    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    g_return_if_fail (MY_IS_WINDOW (toplevel));

    ss = my_window_get_system_settings (MY_WINDOW (toplevel));

    my_system_widget_properties_dialog_show (GTK_WINDOW (toplevel), self);

    gtk_popover_set_relative_to (GTK_POPOVER (ss.popover), priv->button_properties);

    gtk_widget_show (ss.popover);
}

void
my_system_widget_system_changed_size (MySystemWidget * self,
                                      GParamSpec * pspec, gpointer user_data)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    my_system_widget_pixbuf_set_proper_size (self);

    gtk_image_set_from_pixbuf (priv->image, priv->pixbuf_scaled);
}

void
my_system_widget_system_changed (MySystemWidget * self,
                                 GParamSpec * pspec, gpointer user_data)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    g_signal_connect_swapped (priv->system, "notify::width",
                              G_CALLBACK (my_system_widget_system_changed_size),
                              self);
    g_signal_connect_swapped (priv->system, "notify::height",
                              G_CALLBACK (my_system_widget_system_changed_size),
                              self);
}

static void
my_system_widget_init (MySystemWidget * self)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    gulong i, j;

    /* to init any of the private data, do e.g: */

    priv->system = NULL;

    for (i = 0; i < N_MODEL; i++) {
        for (j = 0; j < N_MODEL_HANDLER; j++) {
            priv->model_handler[i][j] = 0;
        }
        priv->model[i] = NULL;
    }

    gtk_widget_init_template (GTK_WIDGET (self));

    g_signal_connect (self, "enter-notify-event",
                      G_CALLBACK (my_system_enter_event), NULL);
    g_signal_connect (self, "leave-notify-event",
                      G_CALLBACK (my_system_leave_event), NULL);
    g_signal_connect (self, "notify::specific-model",
                      G_CALLBACK (my_system_widget_specific_model_added), NULL);
    g_signal_connect (self, "notify::system",
                      G_CALLBACK (my_system_widget_system_changed), NULL);
    g_signal_connect (self, "realize", G_CALLBACK (my_system_widget_realized),
                      NULL);

    g_signal_connect_swapped (priv->button_properties, "clicked",
                              G_CALLBACK
                              (my_system_widget_button_properties_clicked),
                              self);

    GtkWidget *box;

    priv->ib = my_intensity_box_new ();
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_top (box, 10);
    gtk_widget_set_margin_start (box, 40);
    gtk_widget_set_margin_end (box, 40);
    gtk_widget_set_margin_bottom (box, 20);
    gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (priv->ib));
    gtk_widget_show_all (box);

    gtk_stack_add_titled (GTK_STACK (priv->stack), GTK_WIDGET (box), "niveaus",
                          "Niveaus");

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
    gulong i, j;

    priv = my_system_widget_get_instance_private (self);

    data = my_timeline_model_get_systems_data_of_current_pos (timeline);

    if (priv->id <= data->len) {
        model = g_ptr_array_index (data, priv->id);
    }

    g_return_if_fail (MY_IS_SYSTEM_MODEL (model));

    for (i = 0; i < N_MODEL; i++) {
        for (j = 0; j < N_MODEL_HANDLER; j++) {
            if (priv->model_handler[i][j] != 0) {
                g_signal_handler_disconnect (priv->model[i],
                                             priv->model_handler[i][j]);
            }
        }

        g_object_set (self, "specific-model", model, NULL);

        priv->model_handler[i][MODEL_PICTURE_PATH_CHANGED] =
            g_signal_connect_swapped (priv->model[i], "notify::picture-path",
                                      G_CALLBACK
                                      (model_handler_picture_path_changed),
                                      self);

        priv->model_handler[i][MODEL_LABEL_CHANGED] =
            g_signal_connect_swapped (priv->model[i], "notify::label",
                                      G_CALLBACK
                                      (model_handler_label_changed), self);
    }
}

static void
my_system_widget_translate_to_canvas_coordinates (MySystemWidget * self,
                                                  gdouble * x, gdouble * y)
{
    GtkWidget *toplevel;
    gint dest_x, dest_y;
    MyCanvas *canvas;

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    canvas = my_window_get_canvas (MY_WINDOW (toplevel));

    gtk_widget_translate_coordinates (GTK_WIDGET (self), GTK_WIDGET (canvas),
                                      (gint) * x, (gint) * y, &dest_x, &dest_y);

    *x += dest_x;
    *y += dest_y;
}

gboolean
my_system_widget_begin_drag (MySystemWidget * self, GdkEventButton * event,
                             gpointer data)
{
    GtkWidget *toplevel;
    MyCanvas *canvas;

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    canvas = my_window_get_canvas (MY_WINDOW (toplevel));

    my_canvas_begin_drag (GOC_CANVAS (canvas), event, self);

    return GDK_EVENT_STOP;
}

gboolean
my_system_widget_is_dragged (MySystemWidget * self, GdkEventMotion * event,
                             gpointer data)
{
    GtkWidget *toplevel;
    MyCanvas *canvas;

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    canvas = my_window_get_canvas (MY_WINDOW (toplevel));

    my_canvas_is_dragged (GOC_CANVAS (canvas), event, self);

    return GDK_EVENT_STOP;
}

gboolean
my_system_widget_end_drag (MySystemWidget * self, GdkEvent * event,
                           gpointer data)
{

    GtkWidget *toplevel;
    MyCanvas *canvas;

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));

    canvas = my_window_get_canvas (MY_WINDOW (toplevel));

    my_canvas_end_drag (GOC_CANVAS (canvas), event, self);

    return GDK_EVENT_STOP;
}

void
my_system_widget_realized (MySystemWidget * self, gpointer data)
{
    MyTimelineModel *timeline;
    GtkWidget *toplevel;
    MySystem *system;

    GVariant *state;
    gchar *str;
    GAction *action;

    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));
    g_return_if_fail (MY_IS_WINDOW (toplevel));
    timeline = my_window_get_timeline (MY_WINDOW (toplevel));
    g_return_if_fail (MY_IS_TIMELINE_MODEL (timeline));

    g_signal_connect_swapped (timeline, "current-pos-changed",
                              G_CALLBACK
                              (my_system_widget_timeline_current_index_changed),
                              self);

    g_signal_connect (self,
                      "button-press-event",
                      G_CALLBACK (my_system_widget_begin_drag), NULL);

    g_signal_connect (self,
                      "button-release-event",
                      G_CALLBACK (my_system_widget_end_drag), NULL);

    g_signal_connect (self,
                      "motion-notify-event",
                      G_CALLBACK (my_system_widget_is_dragged), NULL);

    my_system_widget_timeline_current_index_changed (self, timeline);


    /* sync with the current visible stack container */
    action =
        g_action_map_lookup_action (G_ACTION_MAP (toplevel), "change-view");

    state = g_action_get_state (action);

    str = g_variant_dup_string (state, NULL);

    gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), str);

    g_free (str);
}

MySystemWidget *
my_system_widget_new (void)
{
    MySystemWidget *self;

    self = g_object_new (MY_TYPE_SYSTEM_WIDGET, NULL);

    return self;
}

void
my_system_widget_set_visible_child (MySystemWidget * self, gchar * name)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    g_return_if_fail (MY_IS_SYSTEM_WIDGET (self));

    gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), name);
}

MyIntensityBox *
my_system_widget_get_intensity_box (MySystemWidget * self)
{
    MySystemWidgetPrivate *priv = my_system_widget_get_instance_private (self);

    g_return_if_fail (MY_IS_SYSTEM_WIDGET (self));

    return priv->ib;
}
