#ifndef RECORD_PAGE_H
#define RECORD_PAGE_H

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

    private:
      Gtk::Window &mainWindow;
      window_selector windowSelector;
      gamepad_selector gamepadSelector;
      Gtk::Button startButton;
      Gtk::Box dd_box;
      Gtk::Label dd_label;
      Gtk::FileChooserButton dd_fileChooser;
      Gtk::Box gl_box;
      record_window *recordWindow;

      void on_startButton_clicked();
      bool on_record_window_closed(GdkEventAny *event);
    };
  }
}

#endif
