#ifndef GAMEPAD_FILE_H
#define GAMEPAD_FILE_H

#include <fstream>
#include <string>
#include <stdexcept>

#include "gamepad.h"

namespace ssbml
{
  class gamepad_file : public gamepad
  {
  public:
    gamepad_file(std::string fileName);
    ~gamepad_file();

    gamepad_file& operator>>(compressed &c);
    gamepad_file& operator>>(uint8_t *buf);

    void get_next();
    void get_next2(compressed &c);
    void rewind();

  private:
    std::ifstream stream;
  };
}

#endif
