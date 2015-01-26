#ifndef __MY_TIMELINE_MODEL_H__
#define __MY_TIMELINE_MODEL_H__

#include <glib-object.h>

#include "my-system.h"


G_BEGIN_DECLS

#define MY_TYPE_TIMELINE_MODEL             (my_timeline_model_get_type())
#define MY_TIMELINE_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_TIMELINE_MODEL,MyTimelineModel))
#define MY_TIMELINE_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_TIMELINE_MODEL,MyTimelineModelClass))
#define MY_IS_TIMELINE_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_TIMELINE_MODEL))
#define MY_IS_TIMELINE_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_TIMELINE_MODEL))
#define MY_TIMELINE_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_TIMELINE_MODEL,MyTimelineModelClass))

#define MY_TIMELINE_MODEL_ERROR                (my_timeline_model_error_quark ())

typedef struct _MyTimelineModel MyTimelineModel;
typedef struct _MyTimelineModelClass MyTimelineModelClass;
typedef struct _MyTimelineModelPrivate MyTimelineModelPrivate;

struct _MyTimelineModel
{
    GObject parent;
    /* insert public members here */

    /* private */
    MyTimelineModelPrivate *_priv;
};

struct _MyTimelineModelClass
{
    GObjectClass parent_class;

};

GType my_timeline_model_get_type (void);

/* fill in public functions */
MyTimelineModel *my_timeline_model_new (void);

G_END_DECLS

#endif /* __MY_TIMELINE_MODEL_H__ */
