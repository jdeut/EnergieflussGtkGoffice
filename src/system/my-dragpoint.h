#ifndef __MY_DRAG_POINT_H__
#define __MY_DRAG_POINT_H__

#include <glib-object.h>
#include <goffice/canvas/goc-circle.h>

#include "my-flowarrow.h"


G_BEGIN_DECLS

#define MY_TYPE_DRAG_POINT             (my_drag_point_get_type())
#define MY_DRAG_POINT(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_DRAG_POINT,MyDragPoint))
#define MY_DRAG_POINT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_DRAG_POINT,MyDragPointClass))
#define MY_IS_DRAG_POINT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_DRAG_POINT))
#define MY_IS_DRAG_POINT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_DRAG_POINT))
#define MY_DRAG_POINT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_DRAG_POINT,MyDragPointClass))

#define MY_DRAG_POINT_ERROR                (my_drag_point_error_quark ())

typedef struct _MyDragPoint MyDragPoint;
typedef struct _MyDragPointClass MyDragPointClass;
typedef struct _MyDragPointPrivate MyDragPointPrivate;

struct _MyDragPoint
{
    GocCircle parent;
    /* insert public members here */

    /* private */
    MyDragPointPrivate *_priv;
};

struct _MyDragPointClass
{
    GocCircleClass parent_class;

};

GType my_drag_point_get_type (void);

/* fill in public functions */

MyDragPoint *my_drag_point_new (void);

void
my_drag_point_begin_dragging (MyDragPoint * self);

gboolean
my_drag_point_is_dragged (MyDragPoint * self);

void
my_drag_point_end_dragging (MyDragPoint * self);

G_END_DECLS

#endif /* __MY_DRAG_POINT_H__ */
