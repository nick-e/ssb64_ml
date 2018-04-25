#ifndef record_page_H
#define record_page_H

#include <fstream>
#include <string>

#include <gtkmm-3.0/gtkmm.h>

#include "record_window.h"
#include "gamepad_selector.h"
#include "window_selector.h"

namespace ssbml
{
  namespace gui
  {
    class record_page : public Gtk::Box
    {
    public:
      record_page(Gtk::Window &mainWindow);

    protected:
      Gtk::Window &mainWindow;
      window_selector windowSelector;
      gamepad_selector gamepadSelector;
      Gtk::Button startButton;
      Gtk::Box dd_box;
      Gtk::Label dd_label;
      Gtk::FileChooserButton dd_fileChooser;
      record_window *recordWindow;

      void on_startButton_clicked();
      bool on_record_window_closed(GdkEventAny *event);
    };
  }
}

#endif
