#include "threadsafe_gamepad.h"

std::string ssbml::threadsafe_gamepad::to_string()
{
  std::lock_guard<std::mutex> lock(m);
  return gamepad::to_string();
}

void ssbml::threadsafe_gamepad::compress(compressed &c)
{
  std::lock_guard<std::mutex> lock(m);
  gamepad::compress(c);
}

void ssbml::threadsafe_gamepad::update(const compressed &c)
{
  std::lock_guard<std::mutex> lock(m);
  gamepad::update(c);
}

ssbml::threadsafe_gamepad::threadsafe_gamepad(float deadzone) :
  gamepad(deadzone)
{

}

ssbml::threadsafe_gamepad::threadsafe_gamepad(threadsafe_gamepad &other)
{
  std::lock_guard<std::mutex> lock(other.m);
  gamepad((gamepad&)other);
}

ssbml::threadsafe_gamepad::threadsafe_gamepad(gamepad &other) :
  gamepad(other)
{

}


ssbml::threadsafe_gamepad::threadsafe_gamepad(const compressed &c)
{
  update(c);
}



ssbml::threadsafe_gamepad::~threadsafe_gamepad()
{

}

ssbml::threadsafe_gamepad& ssbml::threadsafe_gamepad::operator>>(compressed &c)
{
  compress(c);
  return *this;
}

ssbml::threadsafe_gamepad& ssbml::threadsafe_gamepad::operator=(
  threadsafe_gamepad &other)
{
  if (this != &other)
  {
    std::lock_guard<std::mutex> lock(m), otherLock(other.m);
    buttons = other.buttons;
    analogs = other.analogs;
    deadzone = other.deadzone;
  }
  return *this;
}

ssbml::threadsafe_gamepad& ssbml::threadsafe_gamepad::operator=(
  const gamepad &other)
{
  if (this != &other)
  {
    std::lock_guard<std::mutex> lock(m);
    buttons = other.buttons;
    analogs = other.analogs;
    deadzone = other.deadzone;
  }
  return *this;
}

std::ofstream& operator<<(std::ofstream &stream, ssbml::threadsafe_gamepad &g)
{
  ssbml::gamepad::compressed c;
  g.compress(c);
  stream.write((char*)&c, sizeof(c));
  return stream;
}
