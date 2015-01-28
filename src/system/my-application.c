#include "my-application.h"

/*#include <json-glib/json-glib.h>*/

/*#include <json-glib/json-gobject.h>*/

/* 'private'/'protected' functions */
static void my_application_class_init (MyApplicationClass * klass);
static void my_application_init (MyApplication * self);
static void my_application_finalize (GObject *);
static void my_application_dispose (GObject *);

struct _MyApplicationPrivate
{
    GtkWidget *window;
};

G_DEFINE_TYPE_WITH_PRIVATE (MyApplication, my_application, GTK_TYPE_APPLICATION);

static const GActionEntry my_application_app_entries[] = {
    {"quit", my_application_quit, NULL, NULL, NULL}
};


GQuark
my_application_error_quark (void)
{
    return g_quark_from_static_string ("my-application-error-quark");
}

static void
my_application_startup (GApplication * app)
{
    G_APPLICATION_CLASS (my_application_parent_class)->startup (app);

    g_action_map_add_action_entries (G_ACTION_MAP (app),
                                     my_application_app_entries,
                                     G_N_ELEMENTS
                                     (my_application_app_entries), app);
}

static void
my_application_activate (GApplication * app)
{
    MyApplicationPrivate *priv;
    MyApplication *self = MY_APPLICATION (app);

    G_APPLICATION_CLASS (my_application_parent_class)->activate (app);

    priv = my_application_get_instance_private (self);

    priv->window = (GtkWidget *) my_window_new(GTK_APPLICATION(app));

    /*gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(priv->window));*/

    gtk_window_set_role (GTK_WINDOW (priv->window), "MyGtkTraining");

    gtk_widget_show_all (priv->window);

    /* Bring it to the foreground */
    gtk_window_present (GTK_WINDOW (priv->window));
}

static void
my_application_constructed (GObject * object)
{
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

    gobject_class->finalize = my_application_finalize;
    gobject_class->dispose = my_application_dispose;
    /*gobject_class->constructed = constructed;*/

    gapplication_class->startup = my_application_startup;
    gapplication_class->activate = my_application_activate;
}

static void
my_application_init (MyApplication * self)
{
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
my_application_quit (GSimpleAction * simple, GVariant * parameter,
                     gpointer data)
{
    MyApplicationPrivate *priv;
    MyApplication *self = MY_APPLICATION (data);

    priv = my_application_get_instance_private (self);

    if (priv->window != NULL)
        gtk_widget_destroy (priv->window);

    g_application_quit (G_APPLICATION (self));
}
