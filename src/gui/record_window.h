#ifndef RECORD_WINDOW_H
#define RECORD_WINDOW_H

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "../core/record_session.h"

namespace ssbml
{
  namespace gui
  {
    class record_window : public Gtk::Window
    {
    public:
      Gtk::Label recordingLabel;
      Gtk::Label timeLabel;
      Gtk::Label prefixLabel;
      Gtk::Box box;
      Glib::Dispatcher dispatcher;

      record_window(Display *display, ::Window window,
        std::string gamepadDeviceFileName, std::string dstFolder);

      protected:
        record_session recordSession;
        
        void get_record_info();
    };
  }
}

#endif
