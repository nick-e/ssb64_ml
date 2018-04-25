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

#include "gamepad.h"

namespace ssbml
{
  class gamepad_listener : public gamepad
  {
  public:
    std::mutex m;

    gamepad_listener(std::string deviceFileName);
    ~gamepad_listener();

    int get_device_file();
    bool get_btn_mode();
    bool get_quit();
    void compress(compressed &c);
    void decompress(const compressed &c);
    static unsigned long get_all_connected_gamepad_device_file_names(std::string *dest);
    static std::string get_device_name(std::string deviceFileName);

  protected:
    std::atomic<bool> quit;
    std::string deviceFileName;
    std::string deviceName;
    int deviceFile;
    std::thread listenThread;
  };
}

std::ofstream& operator<<(std::ofstream &stream, ssbml::gamepad_listener &g);

#endif
