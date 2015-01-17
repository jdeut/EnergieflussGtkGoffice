gboolean
propagate_motion_notify_event_to_canvas_cb (GtkWidget * widget,
                            GdkEventMotion * event, Interface * iface);
gboolean
propagate_button_release_event_to_canvas_cb (GtkWidget * widget,
                             GdkEvent * event, Interface * iface);
gboolean
propagate_button_press_event_to_canvas_cb (GtkWidget * widget,
                           GdkEventButton * event, Interface * iface);
gboolean
button_press_cb (GtkWidget *widget, GdkEventButton * event,
                 Interface * iface);
gboolean
motion_notify_cb (GtkWidget * widget,
                  GdkEventMotion * event, Interface * iface);
gboolean
button_release_cb (GtkWidget * widget,
                   GdkEvent * event, Interface * iface);
