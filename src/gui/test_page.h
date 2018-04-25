#ifndef TEST_PAGE_H
#define TEST_PAGE_H

#include <gtkmm-3.0/gtkmm.h>

#include "../core/gamepad_spoofer.h"
#include "test_window.h"
#include "window_selector.h"
#include "gamepad_selector.h"

namespace ssbml
{
  namespace gui
  {
    class test_page : public Gtk::Box
    {
    public:
      test_page(Gtk::Window &mainWindow);
      ~test_page();

    protected:
      Gtk::Window &mainWindow;
      Gtk::Box model_box;
      Gtk::Label model_label;
      Gtk::FileChooserButton model_fileChooser;
      window_selector windowSelector;
      gamepad_selector gamepadSelector;
      Gtk::Button spoof_createButton;
      Gtk::Button testButton;
      gamepad_spoofer *gamepadSpoofer;
      test_window *testWindow;

      void create_gamepad_spoofer();
      void on_test_button_clicked();
      bool on_test_window_closed(GdkEventAny *event);
    };
  }
}

#endif
