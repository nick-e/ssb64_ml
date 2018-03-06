#ifndef RECORDWINDOW_H
#define RECORDWINDOW_H

#include <gtkmm-3.0/gtkmm/window.h>
#include <gtkmm-3.0/gtkmm/label.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <pthread.h>
#include <atomic>

#include "../core/Gamepad.h"
#include "../core/VideoInput.h"

namespace SSBML
{
  class RecordWindow : public Gtk::Window
  {
  public:
    Display *display;
    ::Window window;
    Gamepad gamepad;
    VideoInput videoInput;
    std::string destinationFolder;
    pthread_t tid;
    std::atomic<bool> quit;

    RecordWindow(Display *display, ::Window window,
      std::string gamepadDeviceFileName, std::string destinationFolder);

  protected:
    void on_hide_callback();
  };
}

#endif
