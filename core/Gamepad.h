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

namespace SSBML
{
  class Gamepad
  {
  public:
    static unsigned long get_gamepad_device_file_names(std::string *dest);
  };
}

#endif
