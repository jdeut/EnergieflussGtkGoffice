gboolean
propagate_motion_notify_event_to_canvas_cb (GtkWidget * widget,
                            GdkEventMotion * event, App * app);
gboolean
propagate_button_release_event_to_canvas_cb (GtkWidget * widget,
                             GdkEvent * event, App * app);
gboolean
propagate_button_press_event_to_canvas_cb (GtkWidget * widget,
                           GdkEventButton * event, App * app);
gboolean
button_press_cb (GtkWidget *widget, GdkEventButton * event,
                 App * app);
gboolean
motion_notify_cb (GtkWidget * widget,
                  GdkEventMotion * event, App * app);
gboolean
button_release_cb (GtkWidget * widget,
                   GdkEvent * event, App * app);
