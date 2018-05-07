#ifndef GAMEPAD_SPOOFER_H
#define GAMEPAD_SPOOFER_H

#include <linux/uinput.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <iostream>
#include <mutex>

#include "gamepad.h"
#include "threadsafe_gamepad.h"

namespace ssbml
{
  class gamepad_spoofer : public gamepad
  {
  public:
    gamepad_spoofer();
    ~gamepad_spoofer();

    gamepad_spoofer& operator<<(const compressed &c);
    gamepad_spoofer& operator<<(const gamepad &g);
    gamepad_spoofer& operator<<(threadsafe_gamepad &g);

    void update(const gamepad &g);
    void update(threadsafe_gamepad &g);
    void update(const compressed &c) override;

  private:
    int uinput;
    struct input_event syn;

    void on_close();
  };
}

#endif
