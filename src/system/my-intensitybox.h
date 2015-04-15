#ifndef __MY_INTENSITY_BOX_H__
#define __MY_INTENSITY_BOX_H__

#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MY_TYPE_INTENSITY_BOX             (my_intensity_box_get_type())
#define MY_INTENSITY_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_INTENSITY_BOX,MyIntensityBox))
#define MY_INTENSITY_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_INTENSITY_BOX,MyIntensityBoxClass))
#define MY_IS_INTENSITY_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_INTENSITY_BOX))
#define MY_IS_INTENSITY_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_INTENSITY_BOX))
#define MY_INTENSITY_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_INTENSITY_BOX,MyIntensityBoxClass))

#define MY_INTENSITY_BOX_ERROR                (my_intensity_box_error_quark ())

typedef struct _MyIntensityBox MyIntensityBox;
typedef struct _MyIntensityBoxClass MyIntensityBoxClass;
typedef struct _MyIntensityBoxPrivate MyIntensityBoxPrivate;

struct _MyIntensityBox
{
    GtkDrawingArea parent;
    /* insert public members here */

    /* private */
    MyIntensityBoxPrivate *_priv;
};

struct _MyIntensityBoxClass
{
    GtkDrawingAreaClass parent_class;

};

#include "my-window.h"

GType my_intensity_box_get_type (void);

/* fill in public functions */
MyIntensityBox *my_intensity_box_new (void);

G_END_DECLS

#endif /* __MY_INTENSITY_BOX_H__ */
