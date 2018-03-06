#ifndef RECORDSESSION_H
#define RECORDSESSION_H

#include <string>
#include <X11/X.h>
#include <X11/Xlib.h>

#include "Gamepad.h"
#include "VideoInput.h"

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
