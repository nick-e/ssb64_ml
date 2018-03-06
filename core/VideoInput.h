#ifndef VIDEOINPUT_H
#define VIDEOINPUT_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <fstream>

namespace SSBML
{
  class VideoInput
  {
  public:
    unsigned int frameWidth, frameHeight;

    VideoInput(Display *display, Window window, unsigned int frameWidth,
      unsigned int frameHeight);
    ~VideoInput();
    void get_frame(unsigned char *frame);
    static uint32_t get_window_pid(Display *display, Window window);
    static std::string get_window_title(Display *display, Window window);
    static std::string get_process_name(uint32_t pid);
    static Window* get_all_visible_windows(Display **display,
      unsigned long *numWindows);
  protected:
    Display *display;
    Window window;
    XWindowAttributes windowAttributes;
    //unsigned int *preFrame;
  };
}

#endif
