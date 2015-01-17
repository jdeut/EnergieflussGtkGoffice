#ifndef __MY_APPLICATION_H__
#define __MY_APPLICATION_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS
#define MY_TYPE_APPLICATION             (my_application_get_type())
#define MY_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_APPLICATION,MyApplication))
#define MY_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_APPLICATION,MyApplicationClass))
#define MY_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_APPLICATION))
#define MY_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_APPLICATION))
#define MY_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_APPLICATION,MyApplicationClass))
#define MY_APPLICATION_ERROR                (my_application_error_quark ())
typedef struct _MyApplication MyApplication;
typedef struct _MyApplicationClass MyApplicationClass;
typedef struct _MyApplicationPrivate MyApplicationPrivate;

struct _MyApplication
{
    GtkApplication parent;
    /* insert public members here */

    /* private */
    MyApplicationPrivate *_priv;
};

struct _MyApplicationClass
{
    GtkApplicationClass parent_class;

};

GType my_application_get_type (void);

#include "interface.h"

/* fill in public functions */
MyApplication *
my_application_new (void);

void 
my_application_set_window (MyApplication * self, GtkWidget * window);

void
my_application_quit (GSimpleAction * simple, GVariant * parameter,
                     gpointer data);
void
my_application_add_system (GSimpleAction * simple, GVariant * parameter,
                           gpointer );

void
my_application_add_arrow (GSimpleAction * simple, GVariant * parameter,
                          gpointer );

void
my_application_show_drag_points (GSimpleAction * simple, GVariant * parameter,
                                 gpointer );

G_END_DECLS
#endif /* __MY_APPLICATION_H__ */
