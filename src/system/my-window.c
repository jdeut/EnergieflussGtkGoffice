#include "my-window.h"

/* 'private'/'protected' functions */
static void my_window_class_init (MyWindowClass * klass);
static void my_window_init (MyWindow * self);
static void my_window_finalize (GObject *);
static void my_window_dispose (GObject *);

struct _MyWindowPrivate
{
    /* private members go here */
    GtkStatusbar *statusbar1;
    GtkWidget *scrolledwindow1;
    GtkWidget *scale;

    MyTimelineModel *timeline;

    MyCanvas *canvas;
};

G_DEFINE_TYPE_WITH_PRIVATE (MyWindow, my_window, GTK_TYPE_APPLICATION_WINDOW);

static GActionEntry win_entries[] = {
    {"add-arrow", my_window_add_arrow, NULL, NULL, NULL},
    {"add-system", my_window_add_system, NULL, NULL, NULL},
    {"show-drag-points", my_window_show_drag_points, NULL, NULL, NULL},
    {"save", my_window_save, NULL, NULL, NULL},
    {"timeline-add", my_window_timeline_add, NULL, NULL, NULL}
};

GQuark
my_window_error_quark (void)
{
    return g_quark_from_static_string ("my-window-error-quark");
}

void
my_window_style_init (MyWindow * self)
{
    GError *err = NULL;
    GFile *css_file;
    GtkCssProvider *style;
    GdkScreen *screen;

    style = gtk_css_provider_new ();

    css_file = g_file_new_for_uri ("resource:///org/gtk/myapp/style.css");
    gtk_css_provider_load_from_file (style, css_file, &err);

    if (err != NULL) {
        g_printerr ("Error while loading style file: %s\n", err->message);
        g_clear_error (&err);
        /*exit (1); */
    }
    else {

        screen = gtk_window_get_screen (GTK_WINDOW (self));
        gtk_style_context_add_provider_for_screen (screen,
                                                   GTK_STYLE_PROVIDER (style),
                                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    }
}

void
my_window_populate (MyWindow * self)
{
    MyWindowPrivate *priv;

    GocItem *system1;
    GtkAdjustment *adjust;
    MyTimelineModel *timeline;

    priv = my_window_get_instance_private (self);

    priv->canvas = g_object_new (MY_TYPE_CANVAS, "expand", TRUE, NULL);

    gtk_container_add (GTK_CONTAINER (priv->scrolledwindow1),
                       GTK_WIDGET (priv->canvas));

    timeline = my_timeline_model_new ();

    priv->timeline = timeline;

    my_timeline_model_append_to_timeline (timeline);
    my_timeline_model_append_to_timeline (timeline);
    my_timeline_model_append_to_timeline (timeline);

    if (GTK_IS_SCALE (priv->scale)) {
        g_object_bind_property (timeline, "adjustment", priv->scale,
                                "adjustment",
                                G_BINDING_SYNC_CREATE |
                                G_BINDING_BIDIRECTIONAL);
    }

    my_canvas_set_timeline (priv->canvas, timeline);

    system1 = g_object_new (MY_TYPE_SYSTEM, "x", 100.0, "y", 200.0, NULL);

    my_timeline_model_add_object (timeline, system1);
}

static void
my_window_class_init (MyWindowClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_window_finalize;
    gobject_class->dispose = my_window_dispose;

    /* if following line is uncommented and the virtual methode is not chained the
     * program hangs */
    /* gobject_class->constructed = my_window_constructed; */

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
                                                 "/org/gtk/myapp/window.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
                                                  MyWindow, statusbar1);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
                                                  MyWindow, scrolledwindow1);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
                                                  MyWindow, scale);
}

static void
my_window_init (MyWindow * self)
{
    gtk_widget_init_template (GTK_WIDGET (self));

    g_action_map_add_action_entries (G_ACTION_MAP (self),
                                     win_entries,
                                     G_N_ELEMENTS (win_entries), self);
}

static void
my_window_dispose (GObject * object)
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
my_window_new (GtkApplication * app)
{
    MyWindow *self;

    self = g_object_new (MY_TYPE_WINDOW, "application", app, NULL);

    my_window_populate (self);
    my_window_style_init (self);

    return self;
}

void
my_window_add_arrow (GSimpleAction * simple, GVariant * parameter,
                     gpointer data)
{
    MyWindowPrivate *priv;

    priv = my_window_get_instance_private (data);

    guint contextid;
    gchar *msg = { "Click the system to which you want to add an energy flow" };

    contextid = gtk_statusbar_get_context_id (priv->statusbar1, msg);

    gtk_statusbar_push (priv->statusbar1, contextid, msg);

    my_canvas_set_add_arrow_mode (priv->canvas);
}

void
my_window_caution (MyWindow * self, const gchar * error)
{
    GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (self),
                                                GTK_DIALOG_MODAL,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_CLOSE,
                                                error, NULL);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void
my_window_write_file (MyWindow * self, const gchar * path,
                      const gchar * buffer, gssize len)
{
    GError *err = NULL;

    g_file_set_contents (path, buffer, len, &err);

    if (err) {
        gchar *str;

        str = g_strdup_printf ("Failed to save in file '%s'", path);
        my_window_caution (self, str);
        g_free (str);
        g_error_free (err);
    }
}

void
my_window_save (GSimpleAction * simple, GVariant * parameter, gpointer data)
{
    MyWindow *self = MY_WINDOW (data);
    MyWindowPrivate *priv;

    GtkFileFilter *filter;
    GtkWidget *file_chooser;

    gchar *str;
    gchar *path;
    gsize len;

    priv = my_window_get_instance_private (data);

    my_canvas_generate_json_data_stream (priv->canvas, &str, &len);

    file_chooser =
        gtk_file_chooser_dialog_new ("Save", GTK_WINDOW (self),
                                     GTK_FILE_CHOOSER_ACTION_SAVE, "_Cancel",
                                     GTK_RESPONSE_CANCEL, "_Save",
                                     GTK_RESPONSE_ACCEPT, NULL);


    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (file_chooser), TRUE);
    gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (file_chooser), FALSE);

    filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter, "JSON");
    gtk_file_filter_add_pattern (filter, "*.json");

    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser), filter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (file_chooser), filter);

    filter = gtk_file_filter_new ();

    gtk_file_filter_set_name (filter, "All files");
    gtk_file_filter_add_pattern (filter, "*");

    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser), filter);

    gtk_window_set_modal (GTK_WINDOW (file_chooser), TRUE);

    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER
                                                    (file_chooser), TRUE);

    if (gtk_dialog_run (GTK_DIALOG (file_chooser)) == GTK_RESPONSE_ACCEPT) {
        path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
        my_window_write_file (self, path, str, len);
    }

    gtk_widget_destroy (file_chooser);

    g_free (str);
}

void
my_window_show_drag_points (GSimpleAction * simple, GVariant * parameter,
                            gpointer data)
{
    MyWindowPrivate *priv;

    priv = my_window_get_instance_private (data);

    my_canvas_show_drag_points_of_all_arrows (priv->canvas);
}

void
my_window_timeline_add (GSimpleAction * simple, GVariant * parameter,
                        gpointer data)
{
    MyWindowPrivate *priv;

    priv = my_window_get_instance_private (data);
  
    my_timeline_model_add_at_current_pos (priv->timeline);
}

void
my_window_add_system (GSimpleAction * simple, GVariant * parameter,
                      gpointer data)
{
    MyWindowPrivate *priv;

    priv = my_window_get_instance_private (data);

    my_canvas_set_add_system_mode (priv->canvas);
}
