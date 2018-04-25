#ifndef VIDEO_INPUT_H
#define VIDEO_INPUT_H

#include <iostream>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/XShm.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include <cuda.h>
#include <stdio.h>

extern "C"
{
  #include <libavformat/avformat.h>
  #include <libavutil/frame.h>
}

#include "video_output.h"

namespace ssbml
{
  class video_input
  {
  public:
    video_input(Display *display, Window window, uint64_t frameWidth,
      uint64_t frameHeight);
    ~video_input();

    video_input& operator>>(video_output &videoOutput);
    video_input& operator>>(uint8_t *rgbBuf);
    void get_frame(uint8_t *rgbBuf);
    void get_frame(AVFrame &frame);

    static uint32_t get_window_pid(Display *display, Window window);
    static std::string get_window_title(Display *display, Window window);
    static std::string get_process_name(uint32_t pid);
    static Window* get_all_visible_windows(Display **display,
      unsigned long *numWindows);

  protected:
    Display *display;
    Window window;
    uint64_t frameWidth, frameHeight, frameSize, chromaSize;
    XWindowAttributes windowAttributes;
    uint8_t *rgbaBuf, *rgbaBuf1, *rgbaBuf2, *rgbBuf;
    uint8_t *lumaBuf, *redChromaBuf, *blueChromaBuf;

    void get_image();
  };
}

#endif
