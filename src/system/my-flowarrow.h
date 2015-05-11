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

#define ENERGY_FACTOR 1./200.

typedef struct _MyFlowArrow MyFlowArrow;
typedef struct _MyFlowArrowClass MyFlowArrowClass;

struct _MyFlowArrow
{
    GocLine parent;
};

struct _MyFlowArrowClass
{
    GocLineClass parent_class;
};

typedef enum
{
  MY_ANCHOR_NORTH,
  MY_ANCHOR_SOUTH,
  MY_ANCHOR_WEST,
  MY_ANCHOR_EAST,
  N_MY_ANCHORS
} MyAnchorType;

typedef enum
{
    MY_TRANSFER_WORK,
    MY_TRANSFER_HEAT,
    MY_TRANSFER_RADIATION,
    MY_TRANSFER_MASS
} MyTransferTypeEnum;


#include "my-system.h"


#define MY_TYPE_ANCHOR_TYPE     my_anchor_type_get_type ()

GType my_anchor_type_get_type (void);

GType my_flow_arrow_get_type (void);

/* fill in public functions */
void
my_flow_arrow_drag_points_set_visible (MyFlowArrow * self, gboolean visible);

void
my_flow_arrow_drag_points_show (MyFlowArrow * self);

void
my_flow_arrow_drag_points_hide (MyFlowArrow * self);

MyDragPoint *
my_flow_arrow_get_drag_point (MyFlowArrow * self);

void
my_flow_arrow_begin_drag (MyFlowArrow * self);

gboolean
my_flow_arrow_is_dragged (MyFlowArrow * self);

void
my_flow_arrow_end_drag (MyFlowArrow * self);

void
my_flow_arrow_set_coordinate (MyFlowArrow * self, const gchar * first_arg_name, ...);

gdouble
my_flow_arrow_get_preferred_width (MyFlowArrow * self);

gdouble
my_flow_arrow_get_preferred_height (MyFlowArrow * self);

G_END_DECLS

#endif /* __MY_FLOW_ARROW_H__ */
