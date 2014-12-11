#ifndef __MY_FLOW_ARROW_H__
#define __MY_FLOW_ARROW_H__

#include <glib-object.h>
#include <goffice/canvas/goc-line.h>


G_BEGIN_DECLS

#define MY_TYPE_FLOW_ARROW             (my_flow_arrow_get_type())
#define MY_FLOW_ARROW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_FLOW_ARROW,MyFlowArrow))
#define MY_FLOW_ARROW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_FLOW_ARROW,MyFlowArrowClass))
#define MY_IS_FLOW_ARROW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_FLOW_ARROW))
#define MY_IS_FLOW_ARROW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_FLOW_ARROW))
#define MY_FLOW_ARROW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_FLOW_ARROW,MyFlowArrowClass))

#define MY_FLOW_ARROW_ERROR                (my_flow_arrow_error_quark ())

typedef struct _MyFlowArrow MyFlowArrow;
typedef struct _MyFlowArrowClass MyFlowArrowClass;
typedef struct _MyFlowArrowPrivate MyFlowArrowPrivate;

struct _MyFlowArrow
{
    GocLine parent;
    /* insert public members here */

    /* private */
    MyFlowArrowPrivate *_priv;
};

struct _MyFlowArrowClass
{
    GocLineClass parent_class;

};

GType my_flow_arrow_get_type (void);

/* fill in public functions */
MyFlowArrow *my_flow_arrow_new (void);

G_END_DECLS

#endif /* __MY_FLOW_ARROW_H__ */