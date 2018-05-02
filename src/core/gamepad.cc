#include "gamepad.h"

void ssbml::gamepad::compress(compressed &c)
{
  c.buttons = buttons.a | (buttons.b << 1) | (buttons.x << 2)
    | (buttons.y << 3) | (buttons.tr << 4) | (buttons.tl << 5)
    | (buttons.thumbr << 6) | (buttons.thumbl << 7) | (buttons.select << 8)
    | (buttons.start << 9) | (buttons.mode << 10);
  c.analogs = analogs;
}

void ssbml::gamepad::update(const compressed &c)
{
  buttons.a = c.buttons & 0x1;
  buttons.b = (c.buttons >> 1) & 0x1;
  buttons.x = (c.buttons >> 2) & 0x1;
  buttons.y = (c.buttons >> 3) & 0x1;
  buttons.tr = (c.buttons >> 4) & 0x1;
  buttons.tl = (c.buttons >> 5) & 0x1;
  buttons.thumbr = (c.buttons >> 6) & 0x1;
  buttons.thumbl = (c.buttons >> 7) & 0x1;
  buttons.select = (c.buttons >> 8) & 0x1;
  buttons.start = (c.buttons >> 9) & 0x1;
  buttons.mode = (c.buttons >> 10) & 0x1;
  analogs = c.analogs;
}

ssbml::gamepad::gamepad() :
  buttons(
  {
    .a = 0,
    .b = 0,
    .x = 0,
    .y = 0,
    .tr = 0,
    .tl = 0,
    .select = 0,
    .start = 0,
    .mode = 0,
    .thumbl = 0,
    .thumbr = 0
  }),
  analogs(
  {
    .x = 0,
    .y = 0,
    .z = 0,
    .rx = 0,
    .ry = 0,
    .rz = 0,
    .hat0x = 0,
    .hat0y = 0
  })
{

}

ssbml::gamepad& ssbml::gamepad::operator<<(const compressed &c)
{
  update(c);
  return *this;
}

ssbml::gamepad& ssbml::gamepad::operator>>(compressed &c)
{
  compress(c);
  return *this;
}

ssbml::gamepad& ssbml::gamepad::operator=(const gamepad &other)
{
  if (this != &other)
  {
    buttons = other.buttons;
    analogs = other.analogs;
  }
  return *this;
}

ssbml::gamepad::gamepad(const gamepad &other) :
  buttons(other.buttons),
  analogs(other.analogs)
{

}

ssbml::gamepad::gamepad(const compressed &c)
{
  update(c);
}

ssbml::gamepad::~gamepad()
{

}

std::ofstream& operator<<(std::ofstream &stream, ssbml::gamepad &g)
{
  ssbml::gamepad::compressed c;
  g.compress(c);
  stream.write((char*)&c, sizeof(c));
  return stream;
}
