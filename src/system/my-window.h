#ifndef __MY_WINDOW_H__
#define __MY_WINDOW_H__

#include <glib-object.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MY_TYPE_WINDOW             (my_window_get_type())
#define MY_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),MY_TYPE_WINDOW,MyWindow))
#define MY_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),MY_TYPE_WINDOW,MyWindowClass))
#define MY_IS_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),MY_TYPE_WINDOW))
#define MY_IS_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),MY_TYPE_WINDOW))
#define MY_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),MY_TYPE_WINDOW,MyWindowClass))

#define MY_WINDOW_ERROR                (my_window_error_quark ())

enum
{
    UNIT_JOULE,
    UNIT_WATTHOUR,
    UNIT_CAL,
    N_UNIT
};

enum
{
    FACTOR_MILLI,
    FACTOR_ONE,
    FACTOR_KILO,
    FACTOR_MEGA,
    FACTOR_GIGA,
    N_FACTOR
};

typedef struct _MyWindow MyWindow;
typedef struct _MyWindowClass MyWindowClass;
typedef struct _MyWindowPrivate MyWindowPrivate;

typedef struct {
    GtkWidget *button;
    GtkWidget *popover;
    GtkWidget *box;
    GtkWidget *spinbutton;
    GtkComboBox *unit;
    GtkComboBox *prefix;
    GtkAdjustment *adj;
} EnergySettings;

typedef struct {
    GtkWidget *popover;
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *energy_quantity;
    GtkComboBox *transfer_type;
    GtkComboBox *unit;
    GtkComboBox *prefix;
    GtkAdjustment *adj;
} FlowArrowSettings;

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

#include "my-application.h"
#include "my-canvas.h"
#include "my-systemwidget.h"
#include "my-intensitybox.h"

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

MyTimelineModel *
my_window_get_timeline (MyWindow * self);

MyCanvas *
my_window_get_canvas (MyWindow * self);

GtkWidget *
my_window_get_change_view_radio_button (MyWindow * self);

FlowArrowSettings
my_window_get_flow_arrow_settings (MyWindow * self);

gdouble
my_window_get_metric_prefix_factor (MyWindow * self);

G_END_DECLS

#endif /* __MY_WINDOW_H__ */
