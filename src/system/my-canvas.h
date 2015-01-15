#ifndef __MY_CANVAS_H__
#define __MY_CANVAS_H__

#include <glib-object.h>
#include <goffice/canvas/goc-canvas.h>
#include "my-flowarrow.h"


G_BEGIN_DECLS

#define MY_TYPE_CANVAS             (my_canvas_get_type())
#define MY_CANVAS(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_CANVAS,MyCanvas))
#define MY_CANVAS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_CANVAS,MyCanvasClass))
#define MY_IS_CANVAS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_CANVAS))
#define MY_IS_CANVAS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_CANVAS))
#define MY_CANVAS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_CANVAS,MyCanvasClass))

#define MY_CANVAS_ERROR                (my_canvas_error_quark ())

typedef struct _MyCanvas MyCanvas;
typedef struct _MyCanvasClass MyCanvasClass;
typedef struct _MyCanvasPrivate MyCanvasPrivate;

struct _MyCanvas
{
    GocCanvas parent;
    /* insert public members here */
    GocGroup *group_arrows, *group_systems;

    /* private */
    MyCanvasPrivate *_priv;
};

struct _MyCanvasClass
{
    GocCanvasClass parent_class;

};

GType my_canvas_get_type (void);

/* fill in public functions */
MyCanvas *my_canvas_new (void);

void
my_canvas_show_drag_points_of_all_arrows (MyCanvas * self);

gboolean
my_canvas_button_release_cb (GocCanvas * canvas, GdkEvent * event,
                             gpointer data);

gboolean
my_canvas_motion_notify_cb (GocCanvas * canvas, GdkEventMotion * event,
                            gpointer data);

gboolean
my_canvas_button_press_cb (GocCanvas * canvas, GdkEventButton * event,
                           gpointer data);
G_END_DECLS

#endif /* __MY_CANVAS_H__ */
