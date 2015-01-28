#ifndef __MY_SYSTEM_MODEL_H__
#define __MY_SYSTEM_MODEL_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define MY_TYPE_SYSTEM_MODEL             (my_system_model_get_type())
#define MY_SYSTEM_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_SYSTEM_MODEL,MySystemModel))
#define MY_SYSTEM_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_SYSTEM_MODEL,MySystemModelClass))
#define MY_IS_SYSTEM_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_SYSTEM_MODEL))
#define MY_IS_SYSTEM_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_SYSTEM_MODEL))
#define MY_SYSTEM_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_SYSTEM_MODEL,MySystemModelClass))

#define MY_SYSTEM_MODEL_ERROR                (my_system_model_error_quark ())

typedef struct _MySystemModel MySystemModel;
typedef struct _MySystemModelClass MySystemModelClass;
typedef struct _MySystemModelPrivate MySystemModelPrivate;

struct _MySystemModel
{
    GObject parent;
    /* insert public members here */

    /* private */
    MySystemModelPrivate *_priv;
};

struct _MySystemModelClass
{
    GObjectClass parent_class;

};

GType my_system_model_get_type (void);

/* fill in public functions */
MySystemModel *my_system_model_new (void);

G_END_DECLS

#endif /* __MY_SYSTEM_MODEL_H__ */
