#ifndef __INTERFACE__
#define __INTERFACE__

#include <gtk/gtk.h>
#include <goffice/goffice.h>

#define GET_UI_ELEMENT(TYPE, ELEMENT)   TYPE *ELEMENT = (TYPE *) \
                                                interface_get_ui_element(iface, #ELEMENT);

#include "my-canvas.h"

typedef struct interface_
{
    GSList *objects;
    MyCanvas *canvas;
} Interface;


#include "my-application.h"
#include "my-timelinemodel.h"

void 
interface_init (Interface *);

GObject *
interface_get_ui_element (Interface *, const gchar *);

void
interface_create (MyApplication * app);

#include "../dialog-property-editor.h"
#include "my-system.h"
#include "my-flowarrow.h"

#endif
