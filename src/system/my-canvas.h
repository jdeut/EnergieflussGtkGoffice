#ifndef __MY_CANVAS_H__
#define __MY_CANVAS_H__

#include <glib-object.h>
#include <goffice/canvas/goc-canvas.h>

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

enum {
    GROUP_ARROWS,
    GROUP_SYSTEMS,
    GROUP_ARROW_DRAGPOINTS,
    GROUP_SYSTEM_DRAGPOINTS,
    GROUP_LABELS,
    N_GROUPS
};

struct _MyCanvas
{
    GocCanvas parent;
    /* insert public members here */

    GocGroup *group[N_GROUPS];

    /* private */
    MyCanvasPrivate *_priv;
};

struct _MyCanvasClass
{
    GocCanvasClass parent_class;

};

#include "my-timelinemodel.h"
#include "my-flowarrow.h"

GType my_canvas_get_type (void);

/* fill in public functions */
void
my_canvas_set_add_arrow_mode (MyCanvas * self);

void
my_canvas_set_add_system_mode (MyCanvas * self);

void
my_canvas_all_drag_points_set_visible (MyCanvas * self, gboolean visible);

void
my_canvas_all_drag_points_show (MyCanvas * self);

void
my_canvas_all_drag_points_hide (MyCanvas * self);

gboolean
my_canvas_end_drag (GocCanvas * canvas, GdkEvent * event, gpointer data);

gboolean
my_canvas_is_dragged (GocCanvas * canvas, GdkEventMotion * event,
                            gpointer data);

gboolean
my_canvas_begin_drag (GocCanvas * canvas, GdkEventButton * event,
                           gpointer data);

gboolean
my_canvas_generate_json_data_stream (MyCanvas * self, gchar ** str, gsize * len);

void
my_canvas_center_system_bounds (MyCanvas * canvas);

GtkWidget *
my_canvas_get_toplevel (MyCanvas * self);

G_END_DECLS

#endif /* __MY_CANVAS_H__ */
