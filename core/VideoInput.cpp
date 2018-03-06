#include "VideoInput.h"

#include <iostream>

Window* SSBML::VideoInput::get_all_visible_windows(Display **display,
  unsigned long *numWindows)
{
  *display = XOpenDisplay(NULL);
  Atom property = XInternAtom(*display, "_NET_CLIENT_LIST", False), type;
  int form;
  unsigned long remaining;
  unsigned char *list;

  errno = 0;
  if (XGetWindowProperty(*display, XDefaultRootWindow(*display), property, 0,
    1024, False, XA_WINDOW, &type, &form, numWindows, &remaining, &list)
    != Success)
  {
    perror("SSBML::VideoInput::get_all_visible_windows(): XGetWindowProperty()");
    return NULL;
  }

  return (Window*)list;
}

uint32_t SSBML::VideoInput::get_window_pid(Display *display, Window window)
{
  Atom property = XInternAtom(display, "_NET_WM_PID", False), type;
  uint32_t *pidContainer, pid;
  int form;
  unsigned long remaining, bytes;
  if (XGetWindowProperty(display, window, property, 0, 1, False, XA_CARDINAL,
    &type, &form, &bytes, &remaining, (unsigned char**)(&pidContainer))
    != Success)
  {
    perror("SSBML::VideoInput::get_window_pid(): XGetWindowProperty()");
    return 0;
  }
  pid = *pidContainer;
  XFree(pidContainer);
  return pid;
}

std::string SSBML::VideoInput::get_process_name(uint32_t pid)
{
  std::string processName;
  std::ifstream cmdline("/proc/" + std::to_string(pid) + "/cmdline");
  std::getline(cmdline, processName);
  cmdline.close();
  return processName;
}

std::string SSBML::VideoInput::get_window_title(Display *display, Window window)
{
  char *windowTitle;
  if (XFetchName(display, window, &windowTitle))
  {
    std::string tmp(windowTitle);
    XFree(windowTitle);
    return std::string(tmp);
  }
  return "";
}

SSBML::VideoInput::VideoInput(Display *display, Window window,
  unsigned int frameWidth, unsigned int frameHeight) : frameWidth(frameWidth),
  frameHeight(frameHeight), display(display), window(window)
{
  XGetWindowAttributes(display, window, &windowAttributes);

  //preFrame = (unsigned int*)malloc(
  //  sizeof(unsigned int) * windowAttributes.width * windowAttributes.height);
}

SSBML::VideoInput::~VideoInput()
{
  //free(preFrame);
}

static float lerp(float s, float e, float t)
{
  return s + (e - s) * t;
}

static float blerp(float c00, float c10, float c01, float c11, float tx,
  float ty)
{
  return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty);
}

void SSBML::VideoInput::get_frame(unsigned char *frame)
{
  XImage *image = XGetImage(display, window, 0, 0, windowAttributes.width,
    windowAttributes.height, AllPlanes, ZPixmap);
  //std::cout << "writing frame" << std::endl;

  double ratioX = image->width / frameWidth;
  double ratioY = image->height / frameHeight;

  // Bilinear interpolation
  // https://rosettacode.org/wiki/Bilinear_interpolation
  for (int y = 0; y < frameHeight; y++)
  {
    float gy = y / (float)frameHeight * (windowAttributes.height - 1);
    int gyi = (int)gy;
    for (int x = 0; x < frameWidth; x++)
    {
      float gx = x / (float)frameWidth * (windowAttributes.width - 1);
      int gxi = (int)gx;
      unsigned long pixel = XGetPixel(image, gxi, gyi);
      unsigned char red = (pixel & image->red_mask) >> 16;
      unsigned char green = (pixel & image->green_mask) >> 8;
      unsigned char blue = (pixel & image->blue_mask);

      unsigned long c00Pixel = XGetPixel(image, gxi, gyi);
      unsigned char c00Red = (c00Pixel & image->red_mask) >> 16;
      unsigned char c00Green = (c00Pixel & image->green_mask) >> 8;
      unsigned char c00Blue = (c00Pixel & image->blue_mask);

      unsigned long c10Pixel = XGetPixel(image, gxi + 1, gyi);
      unsigned char c10Red = (c10Pixel & image->red_mask) >> 16;
      unsigned char c10Green = (c10Pixel & image->green_mask) >> 8;
      unsigned char c10Blue = (c10Pixel & image->blue_mask);

      unsigned long c01Pixel = XGetPixel(image, gxi, gyi + 1);
      unsigned char c01Red = (c01Pixel & image->red_mask) >> 16;
      unsigned char c01Green = (c01Pixel & image->green_mask) >> 8;
      unsigned char c01Blue = (c01Pixel & image->blue_mask);

      unsigned long c11Pixel = XGetPixel(image, gxi + 1, gyi + 1);
      unsigned char c11Red = (c11Pixel & image->red_mask) >> 16;
      unsigned char c11Green = (c11Pixel & image->green_mask) >> 8;
      unsigned char c11Blue = (c11Pixel & image->blue_mask);

      int index = (y * frameWidth + x) * 3;
      frame[index + 2] = (uint8_t)blerp(c00Red, c10Red, c01Red, c11Red, gx - gxi,
        gy - gyi);
      frame[index + 1] = (uint8_t)blerp(c00Green, c10Green, c01Green, c11Green,
        gx - gxi, gy - gyi);;
      frame[index] = (uint8_t)blerp(c00Blue, c10Blue, c01Blue, c11Blue,
        gx - gxi, gy - gyi);
    }
  }
  /*for (int x = 0, y = 0; y < frameHeight; x++)
  {
    if (x > frameWidth)
    {
      x = 0;
      y++;
    }

    float gx = x / (float)frameWidth * (windowAttributes.width - 1);
    float gy = y / (float)frameHeight * (windowAttributes.height - 1);
    gx = x * ratioX;
    gy = y * ratioY;
    int gxi = (int)gx;
    int gyi = (int)gy;

    unsigned long c00Pixel = XGetPixel(image, gxi, gyi);
    unsigned char c00Red = (c00Pixel & image->red_mask) >> 16;
    unsigned char c00Green = (c00Pixel & image->green_mask) >> 8;
    unsigned char c00Blue = (c00Pixel & image->blue_mask);

    unsigned long c10Pixel = XGetPixel(image, gxi + 1, gyi);
    unsigned char c10Red = (c10Pixel & image->red_mask) >> 16;
    unsigned char c10Green = (c10Pixel & image->green_mask) >> 8;
    unsigned char c10Blue = (c10Pixel & image->blue_mask);

    unsigned long c01Pixel = XGetPixel(image, gxi, gyi + 1);
    unsigned char c01Red = (c01Pixel & image->red_mask) >> 16;
    unsigned char c01Green = (c01Pixel & image->green_mask) >> 8;
    unsigned char c01Blue = (c01Pixel & image->blue_mask);

    unsigned long c11Pixel = XGetPixel(image, gxi + 1, gyi + 1);
    unsigned char c11Red = (c11Pixel & image->red_mask) >> 16;
    unsigned char c11Green = (c11Pixel & image->green_mask) >> 8;
    unsigned char c11Blue = (c11Pixel & image->blue_mask);

    int index = y * frameWidth + x;
    frame[index] = (uint8_t)blerp(c00Red, c10Red, c01Red, c11Red, gx - gxi,
      gy - gyi);
    frame[index + 1] = (uint8_t)blerp(c00Green, c10Green, c01Green, c11Green,
      gx - gxi, gy - gyi);;
    frame[index + 2] = (uint8_t)blerp(c00Blue, c10Blue, c01Blue, c11Blue,
      gx - gxi, gy - gyi);
  }*/

  XFree(image);

  //std::cout << "done" << std::endl;
}
