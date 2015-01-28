#ifndef __MY_WINDOW_H__
#define __MY_WINDOW_H__

#include <glib-object.h>

#include <gtk/gtk.h>

#include "my-canvas.h"
#include "my-application.h"

G_BEGIN_DECLS

#define MY_TYPE_WINDOW             (my_window_get_type())
#define MY_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_WINDOW,MyWindow))
#define MY_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_WINDOW,MyWindowClass))
#define MY_IS_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_WINDOW))
#define MY_IS_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_WINDOW))
#define MY_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_WINDOW,MyWindowClass))

#define MY_WINDOW_ERROR                (my_window_error_quark ())

typedef struct _MyWindow MyWindow;
typedef struct _MyWindowClass MyWindowClass;
typedef struct _MyWindowPrivate MyWindowPrivate;

struct _MyWindow
{
    GtkApplicationWindow parent;
    /* insert public members here */

    /* private */
    MyWindowPrivate *_priv;
};

struct _MyWindowClass
{
    GtkApplicationWindowClass parent_class;

};

GType my_window_get_type (void);

/* fill in public functions */
MyWindow *my_window_new (GtkApplication *app);

void
my_window_save (GSimpleAction * simple, GVariant * parameter, gpointer data);

void
my_window_add_arrow (GSimpleAction * simple, GVariant * parameter,
                     gpointer data);
void
my_window_show_drag_points (GSimpleAction * simple, GVariant * parameter,
                            gpointer data);
void
my_window_add_system (GSimpleAction * simple, GVariant * parameter,
                      gpointer data);

void
my_window_timeline_add (GSimpleAction * simple, GVariant * parameter,
                        gpointer data);
G_END_DECLS

#endif /* __MY_WINDOW_H__ */
