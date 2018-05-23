#ifndef TEST_SESSION_H
#define TEST_SESSION_H

#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "gamepad_listener.h"
#include "video_input.h"
#include "gamepad_spoofer.h"
#include "timer.h"
#include "util.h"
#include "gamepad.h"
#include "child_program.h"

namespace ssbml
{
  class test_session
  {
  public:

    enum class to_child_flag
    {
      test = 0x02,
      clear_lstm_state = 0x03
    };

    struct info
    {
      bool testing;
      bool modelLoaded;
      gamepad::compressed c;
    };

    test_session(Display *display, Window window, uint64_t frameWidth,
      uint64_t frameHeight, double fps, std::string gamepadDeviceFileName,
      std::string modelMeta, gamepad_spoofer &gamepadSpoofer,
      Glib::Dispatcher &dispatcher);
    ~test_session();

    void set_info(const struct info &info);
    void get_info(struct info &info);

  private:
    Glib::Dispatcher &dispatcher;
    double fps;
    uint64_t frameHeight;
    uint64_t frameWidth;
    std::atomic<bool> quit;
    std::string gamepadDeviceFileName;
    std::string modelMeta;
    gamepad_spoofer &gamepadSpoofer;
    Display *display;
    ::Window window;
    struct info info;
    std::mutex m;
    std::thread testThread;

    static void test_thread_routine(test_session &testSession);
  };
}

#endif
