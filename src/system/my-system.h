#ifndef __MY_SYSTEM_H__
#define __MY_SYSTEM_H__

#include <glib-object.h>
#include <goffice/canvas/goc-widget.h>

G_BEGIN_DECLS

#define MY_TYPE_SYSTEM             (my_system_get_type())
#define MY_SYSTEM(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_SYSTEM,MySystem))
#define MY_SYSTEM_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_SYSTEM,MySystemClass))
#define MY_IS_SYSTEM(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_SYSTEM))
#define MY_IS_SYSTEM_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_SYSTEM))
#define MY_SYSTEM_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_SYSTEM,MySystemClass))

#define MY_SYSTEM_ERROR                (my_system_error_quark ())

typedef struct _MySystem MySystem;
typedef struct _MySystemClass MySystemClass;

struct _MySystem
{
    GocWidget parent;
    /* insert public members here */

    /* private */
    GtkListStore *EnergyFlow;
};

struct _MySystemClass
{
    GocWidgetClass parent_class;
};


#include "my-flowarrow.h"
#include "my-canvas.h"
#include "my-timelinemodel.h"


GType my_system_get_type (void);

enum
{
    ANCHOR_NORTH,
    ANCHOR_SOUTH,
    ANCHOR_WEST,
    ANCHOR_EAST,
};

/* fill in public functions */

MyAnchorType
calculate_anchor (MySystem *self, GtkAllocation from, GtkAllocation to);

G_END_DECLS

#endif /* __MY_SYSTEM_H__ */
