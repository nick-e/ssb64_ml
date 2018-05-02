#ifndef GAMEPAD_LISTENER_H
#define GAMEPAD_LISTENER_H

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>

#include "threadsafe_gamepad.h"

namespace ssbml
{
  class gamepad_listener : public threadsafe_gamepad
  {
  public:
    bool get_btn_mode();
    static unsigned long get_all_connected_gamepad_device_file_names(std::string *dest);
    static std::string get_device_name(std::string deviceFileName);

    gamepad_listener(std::string deviceFileName);
    ~gamepad_listener();

  protected:
    std::atomic<bool> quit;
    std::string deviceName;
    int deviceFile;
    std::thread listenThread;

    static void listen_thread_routine(ssbml::gamepad_listener &gl);
  };
}

#endif
