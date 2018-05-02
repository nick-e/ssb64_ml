#ifndef THREADSAFE_GAMEPAD_H
#define THREADSAFE_GAMEPAD_H

#include <mutex>

#include "gamepad.h"

namespace ssbml
{
  class threadsafe_gamepad : public gamepad
  {
  public:
    void compress (compressed &c) override;
    void update(const compressed &c) override;

    threadsafe_gamepad& operator<<(const compressed &c);
    threadsafe_gamepad& operator>>(compressed &c);
    threadsafe_gamepad& operator=(threadsafe_gamepad &other);
    threadsafe_gamepad& operator=(const gamepad &other);

    threadsafe_gamepad();
    threadsafe_gamepad(threadsafe_gamepad &other);
    threadsafe_gamepad(gamepad &other);
    threadsafe_gamepad(const compressed &c);
    ~threadsafe_gamepad() override;

  protected:
    std::mutex m;
  };
}

std::ofstream& operator<<(std::ofstream &stream, ssbml::threadsafe_gamepad &g);

#endif
