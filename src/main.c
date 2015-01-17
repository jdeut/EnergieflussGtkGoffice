#include <goffice/goffice.h>
#include <gtk/gtk.h>

#include "system/my-application.h"

int
main (int argc, char *argv[])
{
    MyApplication *app;
    int status;

    libgoffice_init ();
    
    app = my_application_new();

	status = g_application_run (G_APPLICATION (app), argc, argv);

	g_object_unref (app);

    libgoffice_shutdown ();

	return status;
}
