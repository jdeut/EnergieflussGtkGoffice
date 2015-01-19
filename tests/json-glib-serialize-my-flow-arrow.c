#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

#include "../src/system/my-flowarrow.h"
#include "../src/system/my-system.h"

int
main (int argc, char *argv[])
{

    MyFlowArrow *arrow;
    MySystem *system;

    JsonGenerator *gen = json_generator_new ();
    JsonNode *node, *root;
    JsonArray *array;
    gchar *data;
    gsize len;

    gtk_init (NULL, NULL);
    libgoffice_init ();

    root = json_node_new (JSON_NODE_ARRAY);

    array = json_array_new ();

    arrow =
        g_object_new (MY_TYPE_FLOW_ARROW,
                      "x0", 100.0,
                      "x1", 200.0,
                      "y0", 200.0,
                      "y1", 300.0,
                      "energy-quantity", -22.0, 
                      "anchor", MY_ANCHOR_NORTH,
                      "label-text", "test", NULL);

    node = json_gobject_serialize (G_OBJECT (arrow));
    json_array_add_element (array, node);

    system = g_object_new (MY_TYPE_SYSTEM, "x", 100.0, "y", 200.0, NULL);
    node = json_gobject_serialize (G_OBJECT (system));
    json_array_add_element (array, node);

    json_node_set_array (root, array);

    json_generator_set_root (gen, root);
    json_generator_set_pretty (gen, TRUE);

    data = json_generator_to_data (gen, &len);

    g_print ("%s\n", data);

    libgoffice_shutdown ();

    return 0;
}
