#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <cstdint>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

#include "video_input.h"

namespace ssbml
{
  void create_ppm(std::string path, uint8_t *rgbBuf, uint64_t width,
    uint64_t height);

  std::string time_to_string(unsigned long time);

  int launch_program(std::string scriptName, char * const *args, int *readFd,
    int *writeFd);
}

#endif
