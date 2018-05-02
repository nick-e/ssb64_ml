#ifndef GAMEPAD_FILE_H
#define GAMEPAD_FILE_H

#include <fstream>
#include <string>
#include <stdexcept>

#include "gamepad.h"
#include "gamepad_listener.h"

namespace ssbml
{
  class gamepad_file : public gamepad
  {
  public:
    void get_next();
    void get_next(compressed &c);
    void rewind();

    gamepad_file& operator>>(compressed &c);
    gamepad_file& operator>>(uint8_t *buf);

    gamepad_file(std::string fileName);
    ~gamepad_file();

  private:
    std::ifstream stream;
  };
}

#endif
