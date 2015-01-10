#ifndef __APP__
#define __APP__

#include <gtk/gtk.h>
#include <goffice/goffice.h>

#define UI_DEFINITIONS_FILE "ui.glade"

#define GET_UI_ELEMENT(TYPE, ELEMENT)   TYPE *ELEMENT = (TYPE *) \
                                                app_get_ui_element(app, #ELEMENT);

typedef struct app_
{
    GSList *objects;
    GocCanvas *canvas;
    GocItem *active_item;
} App;

void app_init (App *);

GObject *app_get_ui_element (App *, const gchar *);

#include "callbacks.h"
#include "system/my-system.h"
#include "system/my-flowarrow.h"
#include "dialog-property-editor.h"

#endif
