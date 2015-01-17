#include "my-application.h"

/* 'private'/'protected' functions */
static void my_application_class_init (MyApplicationClass * klass);
static void my_application_init (MyApplication * self);
static void my_application_finalize (GObject *);
static void my_application_dispose (GObject *);

struct _MyApplicationPrivate
{
    GtkWidget *window;
    Interface *iface;
};

#define MY_APPLICATION_GET_PRIVATE(o)      (G_TYPE_INSTANCE_GET_PRIVATE((o), \
                                       MY_TYPE_APPLICATION, \
                                       MyApplicationPrivate))

G_DEFINE_TYPE (MyApplication, my_application, GTK_TYPE_APPLICATION);


static const GActionEntry my_application_app_entries[] = {
    {"quit", my_application_quit, NULL, NULL, NULL}
};

static GActionEntry win_entries[] = {
    {"add-arrow", my_application_add_arrow, NULL, NULL, NULL},
    {"add-system", my_application_add_system, NULL, NULL, NULL},
    {"show-drag-points", my_application_show_drag_points, NULL, NULL, NULL}
};


GQuark
my_application_error_quark (void)
{
    return g_quark_from_static_string ("my-application-error-quark");
}

static void
startup (GApplication * app)
{

    G_APPLICATION_CLASS (my_application_parent_class)->startup (app);
}

static void
activate (GApplication * app)
{
    MyApplication *self = MY_APPLICATION (app);

    /* Create the interface. */
    if (self->_priv->window == NULL) {
        gchar *size_str;

        /* Showtime! */
        interface_create (self);

        g_action_map_add_action_entries (G_ACTION_MAP (app),
                                         my_application_app_entries,
                                         G_N_ELEMENTS
                                         (my_application_app_entries), app);

        g_action_map_add_action_entries (G_ACTION_MAP (self->_priv->window),
                                         win_entries,
                                         G_N_ELEMENTS (win_entries),
                                         self->_priv->iface);

        gtk_window_set_application (GTK_WINDOW (self->_priv->window),
                                    GTK_APPLICATION (self));
        gtk_widget_show_all (self->_priv->window);
    }

    /* Bring it to the foreground */
    gtk_window_present (GTK_WINDOW (self->_priv->window));
}

static void
constructed (GObject * object)
{
    /* Set various properties up */
    /*g_application_set_application_id (G_APPLICATION (object), "org.gnome.Hitori"); */
    /*g_application_set_flags (G_APPLICATION (object), G_APPLICATION_HANDLES_COMMAND_LINE); */

    /* Localisation */
    /*bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR); */
    /*bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8"); */
    /*textdomain (GETTEXT_PACKAGE); */

    g_set_application_name ("Energie");
    gtk_window_set_default_icon_name ("hitori");

    /* Chain up to the parent class */
    G_OBJECT_CLASS (my_application_parent_class)->constructed (object);
}

static void
my_application_class_init (MyApplicationClass * klass)
{
    GObjectClass *gobject_class;
    GApplicationClass *gapplication_class = G_APPLICATION_CLASS (klass);

    gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class, sizeof (MyApplicationPrivate));

    gobject_class->finalize = my_application_finalize;
    gobject_class->dispose = my_application_dispose;
    gobject_class->constructed = constructed;

    gapplication_class->startup = startup;
    gapplication_class->activate = activate;
    /*gapplication_class->command_line = handle_command_line; */
}

static void
my_application_init (MyApplication * self)
{
    self->_priv = MY_APPLICATION_GET_PRIVATE (self);

    /* to init any of the private data, do e.g: */
}

static void
my_application_dispose (GObject * object)
{
    G_OBJECT_CLASS (my_application_parent_class)->dispose (object);
}

static void
my_application_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_application_parent_class)->finalize (object);
}

MyApplication *
my_application_new (void)
{
    MyApplication *self;

    self = g_object_new (MY_TYPE_APPLICATION, NULL);

    return self;
}

void
my_application_set_window (MyApplication * self, GtkWidget * window)
{

    g_return_if_fail (MY_IS_APPLICATION (self));
    g_return_if_fail (GTK_IS_WIDGET (window));

    self->_priv->window = window;
}

void
my_application_set_interface (MyApplication * self, Interface * iface)
{

    g_return_if_fail (MY_IS_APPLICATION (self));

    self->_priv->iface = iface;
}

void
my_application_add_arrow (GSimpleAction * simple, GVariant * parameter,
                          gpointer data)
{
    Interface *iface = (Interface *) data;

    guint contextid;
    gchar *msg = { "Click the system to which you want to add an energy flow" };

    GET_UI_ELEMENT (GtkStatusbar, statusbar1);

    contextid = gtk_statusbar_get_context_id (statusbar1, msg);

    gtk_statusbar_push (statusbar1, contextid, msg);

    my_canvas_add_flow_arrow (iface->canvas);
}

void
my_application_show_drag_points (GSimpleAction * simple, GVariant * parameter,
                                 gpointer data)
{
    Interface *iface = (Interface *) data;

    my_canvas_show_drag_points_of_all_arrows (iface->canvas);
}

void
my_application_add_system (GSimpleAction * simple, GVariant * parameter,
                           gpointer data)
{
    Interface *iface = (Interface *) data;

    my_canvas_add_system (iface->canvas);
}

void
my_application_quit (GSimpleAction * simple, GVariant * parameter,
                     gpointer data)
{
    MyApplication *self = MY_APPLICATION (data);

    if (self->_priv->window != NULL)
        gtk_widget_destroy (self->_priv->window);

    g_application_quit (G_APPLICATION (self));
}
