#ifndef TEST_SESSION_H
#define TEST_SESSION_H

#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <iostream>
#include <iomanip>

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "gamepad_listener.h"
#include "video_input.h"
#include "gamepad_spoofer.h"
#include "timer.h"
#include "util.h"

namespace ssbml
{
  class test_session
  {
  public:
    enum class Py2CC_Flag
    {
      ModelLoaded = 0x41,
      Output = 0x69
    };

    enum class CC2Py_Flag
    {
      Data = 0x33,
      Done = 0x80
    };

    test_session(Display *display, Window window, uint64_t frameWidth,
      uint64_t frameHeight, double fps, std::string gamepadDeviceFileName,
      std::string modelMeta, gamepad_spoofer &gamepadSpoofer,
      Glib::Dispatcher &dispatcher);
    ~test_session();

    bool get_quit();
    void set_test_info(bool testing, bool modelLoaded);
    void get_test_info(bool *testing, bool *modelLoaded);

    protected:
      Glib::Dispatcher &dispatcher;
      std::atomic<bool> quit;
      bool testing;
      bool modelLoaded;
      std::mutex m;
      std::thread testThread;
  };
}

#endif
