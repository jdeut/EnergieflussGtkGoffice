#include "app.h"

static void
dialog_property_editor_response_cb (GtkWidget * dialog,
                                    gint response_id, gpointer * data)
{

    if (response_id == GTK_RESPONSE_OK) {
        g_print ("ok\n");
    }
    if (response_id == GTK_RESPONSE_CANCEL) {
        g_print ("cancel\n");
    }
    gtk_widget_destroy (dialog);
}

void
my_flow_arrow_destroy_clicked_cb (GtkButton * button, MyFlowArrow * flowarrow)
{
    MySystem *system;

    system = my_flow_arrow_get_linked_system (flowarrow);

    my_system_remove_flow_arrow (system, flowarrow);
}

void
dialog_property_editor (GObject * object, gchar * label, GtkWindow * window)
{

    GtkBuilder *builder;
    GtkWidget *dialog, *listbox;
    GOEditor *editor;

    builder = gtk_builder_new_from_file ("./dialog-property-editor.ui");

    dialog = (GtkWidget *) gtk_builder_get_object (builder, "dialog1");
    listbox = (GtkWidget *) gtk_builder_get_object (builder, "listbox1");

    gtk_window_set_transient_for (GTK_WINDOW (dialog), window);

    g_signal_connect (G_OBJECT (dialog), "response",
                      G_CALLBACK (dialog_property_editor_response_cb), NULL);

    if (MY_IS_FLOW_ARROW (object)) {

        gtk_window_set_title (GTK_WINDOW (dialog), "Edit properties of Arrow");

        GtkWidget *hbox, *button, *label, *entry;

        /* add line to change energy quantity */
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);

        g_object_set (hbox, "margin-top", 10, "margin-bottom", 10,
                      "margin-left", 10, "margin-right", 10, NULL);

        button = gtk_spin_button_new_with_range (1, 200, 1);
        label = gtk_label_new ("Energiemenge");

        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, FALSE, 0);

        gtk_list_box_prepend (GTK_LIST_BOX (listbox), hbox);

        g_object_bind_property (object, "energy-quantity", button, "value",
                                G_BINDING_BIDIRECTIONAL |
                                G_BINDING_SYNC_CREATE);

        /* add line to change label */
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);

        g_object_set (hbox, "margin-top", 10, "margin-bottom", 10,
                      "margin-left", 10, "margin-right", 10, NULL);

        entry = gtk_entry_new ();
        label = gtk_label_new ("Beschriftung");

        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);

        gtk_list_box_prepend (GTK_LIST_BOX (listbox), hbox);

        g_object_bind_property (object, "label-text", entry, "text",
                                G_BINDING_BIDIRECTIONAL |
                                G_BINDING_SYNC_CREATE);

        /* add line to destroy arrow */
        hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);

        g_object_set (hbox, "margin-top", 10, "margin-bottom", 10,
                      "margin-left", 10, "margin-right", 10, NULL);

        button = gtk_button_new_with_label ("destroy");
        label = gtk_label_new ("Beschriftung");

        gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

        gtk_list_box_prepend (GTK_LIST_BOX (listbox), hbox);

        g_signal_connect (button, "clicked",
                          G_CALLBACK (my_flow_arrow_destroy_clicked_cb),
                          object);

        gtk_widget_show_all (listbox);
    }

    gtk_widget_show (dialog);
}
