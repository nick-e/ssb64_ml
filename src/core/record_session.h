#ifndef RECORD_SESSION_H
#define RECORD_SESSION_H

#include <string>
#include <thread>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <mutex>
#include <atomic>
#include <stdexcept>

#include <gtkmm-3.0/gtkmm.h>
#include <X11/X.h>
#include <X11/Xlib.h>

/*extern "C"
{
  #include <libavformat/avformat.h>
}*/

#include "gamepad_listener.h"
#include "video_input.h"
#include "video_output.h"
#include "timer.h"

namespace ssbml
{
  class record_session
  {
  public:
    record_session(Display *display, Window window, uint64_t frameWidth,
      uint64_t frameHeight, double fps, std::string gamepadDeviceFileName,
      std::string dstDir, Glib::Dispatcher &dispatcher);
    ~record_session();

    void set_record_info(bool recording, unsigned long totalTime,
      std::string filePrefix);
    void get_record_info(bool *recording, unsigned long *totalTime,
      std::string &filePrefix);
    bool get_quit();

  protected:
    Glib::Dispatcher &dispatcher;
    std::atomic<bool> quit;
    bool recording;
    unsigned long totalTime;
    std::string filePrefix;
    std::mutex m;
    std::thread recordThread;
  };
}

#endif
