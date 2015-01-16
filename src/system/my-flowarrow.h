#ifndef __MY_FLOW_ARROW_H__
#define __MY_FLOW_ARROW_H__

#include <glib-object.h>
#include <goffice/canvas/goc-line.h>

#include "my-dragpoint.h"

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

typedef enum
{
  MY_ANCHOR_CENTER,
  MY_ANCHOR_NORTH,
  MY_ANCHOR_NORTH_WEST,
  MY_ANCHOR_NORTH_EAST,
  MY_ANCHOR_SOUTH,
  MY_ANCHOR_SOUTH_WEST,
  MY_ANCHOR_SOUTH_EAST,
  MY_ANCHOR_WEST,
  MY_ANCHOR_EAST,
  MY_ANCHOR_BASELINE_CENTER,
  MY_ANCHOR_BASELINE_WEST,
  MY_ANCHOR_BASELINE_EAST,
  MY_ANCHOR_N		= MY_ANCHOR_NORTH,
  MY_ANCHOR_NW		= MY_ANCHOR_NORTH_WEST,
  MY_ANCHOR_NE		= MY_ANCHOR_NORTH_EAST,
  MY_ANCHOR_S		= MY_ANCHOR_SOUTH,
  MY_ANCHOR_SW		= MY_ANCHOR_SOUTH_WEST,
  MY_ANCHOR_SE		= MY_ANCHOR_SOUTH_EAST,
  MY_ANCHOR_W		= MY_ANCHOR_WEST,
  MY_ANCHOR_E		= MY_ANCHOR_EAST,
  MY_ANCHOR_B		= MY_ANCHOR_BASELINE_CENTER,
  MY_ANCHOR_BW		= MY_ANCHOR_BASELINE_WEST,
  MY_ANCHOR_BE		= MY_ANCHOR_BASELINE_EAST
} MyAnchorType;

#include "my-system.h"


GType my_flow_arrow_get_type (void);

/* fill in public functions */
void
my_flow_arrow_set_linked_system (MyFlowArrow * self, MySystem * system);

MySystem *
my_flow_arrow_get_linked_system (MyFlowArrow * self);

void
my_flow_arrow_show_drag_points (MyFlowArrow * self);

void
my_flow_arrow_hide_drag_points (MyFlowArrow * self);

void
my_flow_arrow_begin_dragging (MyFlowArrow * self);

gboolean
my_flow_arrow_is_dragged (MyFlowArrow * self);

void
my_flow_arrow_end_dragging (MyFlowArrow * self);

MyDragPoint *
my_flow_arrow_get_drag_point (MyFlowArrow * self);
G_END_DECLS

#endif /* __MY_FLOW_ARROW_H__ */
