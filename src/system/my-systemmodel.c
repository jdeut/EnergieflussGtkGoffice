#include "my-systemmodel.h"

/* 'private'/'protected' functions */
static void my_system_model_class_init (MySystemModelClass * klass);
static void my_system_model_init (MySystemModel * self);
static void my_system_model_finalize (GObject * );
static void my_system_model_dispose (GObject * );

enum
{
    SIG_MODEL_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0, };

enum
{
    PROP_0,
    PROP_PICTURE_PATH,
    PROP_PIXBUF,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

struct _MySystemModelPrivate
{
    /* private members go here */
    gchar *name;

    gchar *picture_path;
    GdkPixbuf *pixbuf;
    guint id;
};

G_DEFINE_TYPE_WITH_PRIVATE (MySystemModel, my_system_model, G_TYPE_OBJECT);

GQuark
my_system_model_error_quark (void)
{
  return g_quark_from_static_string ("my-system-model-error-quark");
}

static void
my_system_model_set_property (GObject * object,
                               guint property_id,
                               const GValue * value, GParamSpec * pspec)
{
    MySystemModel *self = MY_SYSTEM_MODEL (object);

    MySystemModelPrivate *priv = my_system_model_get_instance_private (self);

    switch (property_id) {

        case PROP_PICTURE_PATH:
            priv->picture_path = g_value_dup_string (value);
            break;

        case PROP_PIXBUF:
            priv->pixbuf = g_value_get_object (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void
my_system_model_get_property (GObject * object,
                               guint property_id, GValue * value,
                               GParamSpec * pspec)
{
    MySystemModel *self = MY_SYSTEM_MODEL (object);

    MySystemModelPrivate *priv = my_system_model_get_instance_private (self);

    switch (property_id) {

        case PROP_PICTURE_PATH:
            g_value_set_string (value, priv->picture_path);
            break;

        case PROP_PIXBUF:
            g_value_set_object (value, priv->pixbuf);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}
static void
my_system_model_class_init (MySystemModelClass * klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = my_system_model_finalize;
    gobject_class->dispose = my_system_model_dispose;

    gobject_class->set_property = my_system_model_set_property;
    gobject_class->get_property = my_system_model_get_property;

    signals[SIG_MODEL_CHANGED] =
        g_signal_new ("model-changed",
                      G_OBJECT_CLASS_TYPE (gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    obj_properties[PROP_PICTURE_PATH] =
        g_param_spec_string ("picture-path",
                           "picture-path",
                           "file path of picture",
                           NULL,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

    obj_properties[PROP_PIXBUF] =
        g_param_spec_object ("pixbuf",
                           "pixbuf",
                           "pxel buffer",
                           GDK_TYPE_PIXBUF,
                           G_PARAM_READWRITE);

    g_object_class_install_properties (gobject_class,
                                       N_PROPERTIES, obj_properties);
}

/*void*/
/*my_system_model_picture_path_changed (MySystemModel * self,*/
                                /*GParamSpec * pspec, gpointer user_data)*/
/*{*/
    /*GError *err = NULL;*/
    /*MySystemModelPrivate *priv;*/

    /*priv = my_system_model_get_instance_private(self);*/

    /*if(priv->picture_path != NULL) {*/

        /*priv->pixbuf =*/
            /*gdk_pixbuf_new_from_file_at_scale (priv->picture_path,*/
                                                   /*200, -1, TRUE, &err);*/

        /*if (err) {*/
            /*gchar *str;*/

            /*str = g_strdup_printf ("Failed to load file '%s' into pixbuf", priv->picture_path);*/

            /*g_print("%s\n", str);*/
            /*g_free (str);*/
            /*g_error_free (err);*/

            /*priv->pixbuf = NULL;*/
        /*}*/

        /*if(priv->pixbuf != NULL) {*/
            /*g_signal_emit (G_OBJECT (self), signals[SIG_MODEL_CHANGED], 0);*/
        /*}*/
    /*}*/
/*}*/

static void
my_system_model_init (MySystemModel * self)
{
    MySystemModelPrivate *priv;

    priv = my_system_model_get_instance_private(self);

    /* to init any of the private data, do e.g: */

    priv->pixbuf == NULL;
}

static void
my_system_model_dispose (GObject *object)
{
    G_OBJECT_CLASS (my_system_model_parent_class)->dispose (object);
}

static void
my_system_model_finalize (GObject * object)
{
    /* free/unref instance resources here */
    G_OBJECT_CLASS (my_system_model_parent_class)->finalize (object);
}

MySystemModel *
my_system_model_new (void)
{
    MySystemModel *self;

    self = g_object_new (MY_TYPE_SYSTEM_MODEL, NULL);

    return self;
}
