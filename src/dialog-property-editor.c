#include <gtk/gtk.h>

#include "system/my-flowarrow.h"

static GtkWidget *preferences_dialog = NULL;

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
    GtkDialog *dialog;

    dialog = (GtkDialog *) gtk_widget_get_toplevel (GTK_WIDGET (button));

    if (MY_IS_FLOW_ARROW (flowarrow)) {
        goc_item_destroy (GOC_ITEM (flowarrow));
    }

    if (GTK_IS_DIALOG (dialog)) {
        gtk_widget_destroy (GTK_WIDGET (dialog));
    }
}

void my_flow_arrow_transfer_type_row_changed
    (GtkTreeModel * tree_model,
     GtkTreePath * path, GtkTreeIter * iter, gpointer user_data)
{
    g_print ("ok\n");
}

void
my_flow_arrow_combo_transfer_type_row_changed (GtkWidget * box,
                                               MyFlowArrow * arrow)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    guint nr;

    g_return_if_fail (MY_IS_FLOW_ARROW (arrow));

    gtk_combo_box_get_active_iter (GTK_COMBO_BOX (box), &iter);
    model = gtk_combo_box_get_model (GTK_COMBO_BOX (box));

    gtk_tree_model_get (model, &iter, 1, &nr, -1);

    g_object_set (arrow, "transfer-type", nr, NULL);
}

void
dialog_property_editor (GObject * object, gchar * label, GtkWindow * window)
{
    GtkBuilder *builder;
    GtkWidget *dialog, *listbox;
    GtkWidget *spinbutton_energy_quantity;
    GtkWidget *entry_label_text;
    GtkWidget *button_destroy;
    GtkWidget *combobox_transfer_type;
    GtkListStore *liststore_transfer_type;
    GOEditor *editor;

    if (preferences_dialog != NULL) {
        gtk_window_present (GTK_WINDOW (preferences_dialog));
        return;
    }

    builder =
        gtk_builder_new_from_resource
        ("/org/gtk/myapp/dialog-property-editor.ui");

    dialog = (GtkWidget *) gtk_builder_get_object (builder, "dialog1");
    spinbutton_energy_quantity =
        (GtkWidget *) gtk_builder_get_object (builder, "spinbutton1");
    entry_label_text = (GtkWidget *) gtk_builder_get_object (builder, "entry2");
    button_destroy = (GtkWidget *) gtk_builder_get_object (builder, "button1");
    liststore_transfer_type =
        (GtkListStore *) gtk_builder_get_object (builder, "liststore1");
    combobox_transfer_type =
        (GtkWidget *) gtk_builder_get_object (builder, "combobox1");

    gtk_window_set_transient_for (GTK_WINDOW (dialog), window);

    g_signal_connect (G_OBJECT (dialog), "response",
                      G_CALLBACK (dialog_property_editor_response_cb), NULL);

    if (MY_IS_FLOW_ARROW (object)) {

        guint type, n;
        GtkTreeIter iter;
        GtkTreeModel *model;
        gboolean valid;

        model =
            gtk_combo_box_get_model (GTK_COMBO_BOX (combobox_transfer_type));

        valid = gtk_tree_model_get_iter_first (model, &iter);

        /* sync active iter of combobox with current transfer-type */
        while (valid) {
            g_object_get (object, "transfer-type", &type, NULL);

            gtk_tree_model_get (model, &iter, 1, &n, -1);

            if (n == type) {
                gtk_combo_box_set_active_iter (GTK_COMBO_BOX
                                               (combobox_transfer_type), &iter);
                break;
            }
            valid = gtk_tree_model_iter_next (model, &iter);
        }

        g_object_bind_property (object, "energy-quantity",
                                spinbutton_energy_quantity, "value",
                                G_BINDING_BIDIRECTIONAL |
                                G_BINDING_SYNC_CREATE);

        g_object_bind_property (object, "label-text", entry_label_text, "text",
                                G_BINDING_BIDIRECTIONAL |
                                G_BINDING_SYNC_CREATE);

        g_signal_connect (button_destroy, "clicked",
                          G_CALLBACK (my_flow_arrow_destroy_clicked_cb),
                          object);

        g_signal_connect (liststore_transfer_type, "row-changed",
                          G_CALLBACK (my_flow_arrow_transfer_type_row_changed),
                          object);

        g_signal_connect (combobox_transfer_type, "changed",
                          G_CALLBACK
                          (my_flow_arrow_combo_transfer_type_row_changed),
                          object);

    }

    gtk_widget_show (dialog);

    preferences_dialog = dialog;

    g_object_add_weak_pointer (G_OBJECT (dialog),
                               (gpointer *) & preferences_dialog);

    g_object_unref (builder);

}
