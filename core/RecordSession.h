#ifndef RECORDSESSION_H
#define RECORDSESSION_H

#include <string>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "Gamepad.h"
#include "VideoInput.h"

#define FRAME_WIDTH 256
#define FRAME_HEIGHT 144

namespace SSBML
{
  class RecordSession
  {
  public:
    Display *display;
    Window window;
    Gamepad gamepad;
    VideoInput videoInput;
    std::string destinationFolder;
    std::atomic<bool> quit;

    RecordSession(Display *display, Window window,
      std::string gamepadDeviceFileName, std::string destinationFolder);
    void init();
    void end();

  protected:
    pthread_t tid;
  };
}

#endif
