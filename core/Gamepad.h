#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <iostream>
#include <string>

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <atomic>
#include <pthread.h>

namespace SSBML
{
  class Gamepad
  {
  public:
    enum class EventCode: int
    {
      A = 304,
      B = 305,
      X = 307,
      Y = 308,
      RB = 311,
      LB = 310,
      Select = 314,
      Start = 315,
      Xbox = 316,
      RT = 5,
      LT = 2,
      DPadY = 17,
      DPadX = 16,
      LAnalogX = 0,
      RAnalogX = 3,
      LAnalogY = 1,
      RAnalogY = 4,
      LToggle = 317,
      RToggle = 318
    };

    std::atomic<bool> A, B, X, Y, RB, LB, select, start, xbox, ltoggle, rtoggle, quit;
    std::atomic<int16_t> RT, LT, dpadY, dpadX, lanalogX, lanalogY, ranalogX, ranalogY;

    Gamepad(std::string deviceFileName);
    int init();
    int end();
    int get_device_file_fd();
    void gamepad_binary(char *bin);

    static unsigned long get_all_connected_gamepad_device_file_names(std::string *dest);
    static std::string get_device_name(std::string deviceFileName);

  protected:
    std::string deviceFileName;
    std::string deviceFilePath;
    std::string deviceName;
    int deviceFileFd;
    pthread_t tid;
  };
}

#endif
