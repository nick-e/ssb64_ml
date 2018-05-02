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
    void get_record_info(bool *recording, unsigned long *totalTime,
      std::string &filePrefix);

    record_session(Display *display, Window window, uint64_t frameWidth,
      uint64_t frameHeight, double fps, std::string gamepadDeviceFileName,
      std::string dstDir, Glib::Dispatcher &dispatcher);
    ~record_session();

  protected:
    Glib::Dispatcher &dispatcher;
    bool recording;
    double fps;
    uint64_t frameHeight;
    uint64_t frameWidth;
    unsigned long totalTime;
    std::atomic<bool> quit;
    std::string dstDir;
    std::string filePrefix;
    std::string gamepadDeviceFileName;
    ::Window window;
    Display *display;
    std::mutex m;
    std::thread recordThread;

    void set_record_info(bool recording, unsigned long totalTime,
      std::string filePrefix);
    static void record_thread_routine(record_session &recordSession);
  };
}

#endif
