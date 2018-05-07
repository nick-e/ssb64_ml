#ifndef CHILD_PROGRAM_H
#define CHILD_PROGRAM_H

#include <string>
#include <stdexcept>
#include <vector>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>

namespace ssbml
{
  class child_program
  {
  public:
    enum class to_child_flag
    {
      shutdown_request = 0x00
    };

    enum class from_child_flag
    {
      child_initialized = 0x00,
      child_terminated = 0x01
    };

    ssize_t write_to(uint8_t flag);
    ssize_t write_to(uint8_t *buf, uint64_t length);
    ssize_t try_read_from(uint8_t *buf, uint64_t maxLength);

    child_program(std::string name, std::vector<char*> &args,
      bool waitForInit = true);
    ~child_program();

  private:
    int readFd;
    int writeFd;
  };
}

#endif
