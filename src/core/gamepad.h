#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <cstdint>
#include <linux/input.h>
#include <fstream>
#include <string>

#define ANALOG_MAX_VALUE 32767
#define DEFAULT_DEAD_ZONE (uint16_t)(ANALOG_MAX_VALUE * 0.1)

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
    uint16_t deadzone;

    virtual std::string to_string();
    virtual void compress(compressed &c);
    virtual void update(const compressed &c);

    gamepad& operator<<(const compressed &c);
    gamepad& operator>>(compressed &c);
    gamepad& operator=(const gamepad &other);

    gamepad(uint16_t deadzone = DEFAULT_DEAD_ZONE);
    gamepad(const gamepad& other);
    gamepad(const compressed &c);
    virtual ~gamepad();
  };
}

std::ofstream& operator<<(std::ofstream &stream, ssbml::gamepad &g);

#endif
