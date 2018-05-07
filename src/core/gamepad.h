#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <cstdint>
#include <linux/input.h>
#include <fstream>
#include <string>

namespace ssbml
{
  class gamepad
  {
  public:
    struct buttons
    {
      bool a, b, x, y, tr, tl, thumbr, thumbl, select, start, mode;

      std::string to_string();
    };

    struct analogs
    {
      int16_t x, y, z, rx, ry, rz, hat0x, hat0y;

      std::string to_string();
    };

    struct compressed
    {
      uint16_t buttons;
      analogs analogs;

      std::string to_string();
    };

    buttons buttons;
    analogs analogs;

    virtual std::string to_string();
    virtual void compress(compressed &c);
    virtual void update(const compressed &c);

    gamepad& operator<<(const compressed &c);
    gamepad& operator>>(compressed &c);
    gamepad& operator=(const gamepad &other);

    gamepad();
    gamepad(const gamepad& other);
    gamepad(const compressed &c);
    virtual ~gamepad();
  };
}

std::ofstream& operator<<(std::ofstream &stream, ssbml::gamepad &g);

#endif
