#ifndef __MY_SYSTEM_WIDGET_H__
#define __MY_SYSTEM_WIDGET_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "my-systemmodel.h"
#include "my-timelinemodel.h"
#include "my-window.h"
#include "my-intensitybox.h"

G_BEGIN_DECLS

#define MY_TYPE_SYSTEM_WIDGET             (my_system_widget_get_type())
#define MY_SYSTEM_WIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_SYSTEM_WIDGET,MySystemWidget))
#define MY_SYSTEM_WIDGET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_SYSTEM_WIDGET,MySystemWidgetClass))
#define MY_IS_SYSTEM_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_SYSTEM_WIDGET))
#define MY_IS_SYSTEM_WIDGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_SYSTEM_WIDGET))
#define MY_SYSTEM_WIDGET_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_SYSTEM_WIDGET,MySystemWidgetClass))

#define MY_SYSTEM_WIDGET_ERROR                (my_system_widget_error_quark ())

typedef struct _MySystemWidget MySystemWidget;
typedef struct _MySystemWidgetClass MySystemWidgetClass;
typedef struct _MySystemWidgetPrivate MySystemWidgetPrivate;

struct _MySystemWidget
{
    GtkEventBox parent;
    /* insert public members here */

    /* private */
    MySystemWidgetPrivate *_priv;
};

struct _MySystemWidgetClass
{
    GtkEventBoxClass parent_class;

};

#include "my-systemwidget-properties.h"

GType my_system_widget_get_type (void);

/* fill in public functions */
MySystemWidget *my_system_widget_new (void);

void
my_system_widget_set_visible_child (MySystemWidget * self, gchar *name);

MyIntensityBox *
my_system_widget_get_intensity_box (MySystemWidget * self);

G_END_DECLS

#endif /* __MY_SYSTEM_WIDGET_H__ */
