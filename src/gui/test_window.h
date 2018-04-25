#ifndef TEST_WINDOW_H
#define TEST_WINDOW_H

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "../core/gamepad_spoofer.h"
#include "../core/test_session.h"

namespace ssbml
{
  namespace gui
  {
    class test_window : public Gtk::Window
    {
    public:
      Gtk::Label label;
      Glib::Dispatcher dispatcher;

      test_window(Display *display, ::Window window,
        uint64_t frameWidth, uint64_t frameHeight, double fps,
        std::string gamepadDeviceFileName, std::string modelMeta,
        gamepad_spoofer &gamepadSpoofer);

      private:
        test_session testSession;

        void get_test_info();
    };
  }
}

#endif
