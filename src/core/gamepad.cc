#include "gamepad.h"

std::string ssbml::gamepad::compressed::to_string()
{
  struct buttons buttons;
  buttons.a = this->buttons & 0x1;
  buttons.b = (this->buttons >> 1) & 0x1;
  buttons.x = (this->buttons >> 2) & 0x1;
  buttons.y = (this->buttons >> 3) & 0x1;
  buttons.tr = (this->buttons >> 4) & 0x1;
  buttons.tl = (this->buttons >> 5) & 0x1;
  buttons.thumbr = (this->buttons >> 6) & 0x1;
  buttons.thumbl = (this->buttons >> 7) & 0x1;
  buttons.select = (this->buttons >> 8) & 0x1;
  buttons.start = (this->buttons >> 9) & 0x1;
  buttons.mode = (this->buttons >> 10) & 0x1;

  return buttons.to_string() + ' ' + analogs.to_string();
}

std::string ssbml::gamepad::buttons::to_string()
{
  return std::to_string(a) + ' ' + std::to_string(b) + ' ' + std::to_string(x)
    + ' ' + std::to_string(y) + ' ' + std::to_string(tr) + ' '
    + std::to_string(tl) + ' ' + std::to_string(thumbr) + ' '
    + std::to_string(thumbl) + ' ' + std::to_string(select) + ' '
    + std::to_string(start) + ' ' + std::to_string(mode);
}

std::string ssbml::gamepad::analogs::to_string()
{
  return std::to_string(x) + ' ' + std::to_string(y) + ' ' + std::to_string(z)
    +  ' ' + std::to_string(rx) + ' ' + std::to_string(ry) + ' '
    + std::to_string(rz) + ' ' + std::to_string(hat0x) + ' '
    + std::to_string(hat0y);
}

std::string ssbml::gamepad::to_string()
{
   return buttons.to_string() + ' ' + analogs.to_string();
}

void ssbml::gamepad::compress(compressed &c)
{
  c.buttons = buttons.mode ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.start ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.select ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.thumbl ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.thumbr ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.tl ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.tr ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.y ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.x ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.b ? 1 : 0;
  c.buttons <<= 1;
  c.buttons |= buttons.a ? 1 : 0;
  c.analogs = analogs;
}

void ssbml::gamepad::update(const compressed &c)
{
  buttons.a = c.buttons & 0x1;
  buttons.b = (c.buttons >> 1) & 1;
  buttons.x = (c.buttons >> 2) & 1;
  buttons.y = (c.buttons >> 3) & 1;
  buttons.tr = (c.buttons >> 4) & 1;
  buttons.tl = (c.buttons >> 5) & 1;
  buttons.thumbr = (c.buttons >> 6) & 1;
  buttons.thumbl = (c.buttons >> 7) & 1;
  buttons.select = (c.buttons >> 8) & 1;
  buttons.start = (c.buttons >> 9) & 1;
  buttons.mode = (c.buttons >> 10) & 1;
  analogs = c.analogs;
  if (analogs.x <= deadzone && analogs.x >= -deadzone)
  {
    analogs.x = 0;
  }
  if (analogs.y <= deadzone && analogs.y >= -deadzone)
  {
    analogs.y = 0;
  }
  if (analogs.rx <= deadzone && analogs.rx >= -deadzone)
  {
    analogs.rx = 0;
  }
  if (analogs.ry <= deadzone && analogs.ry >= -deadzone)
  {
    analogs.ry = 0;
  }
}

ssbml::gamepad::gamepad(uint16_t deadzone) :
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
  }),
  deadzone(deadzone)
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
    deadzone = other.deadzone;
  }
  return *this;
}

ssbml::gamepad::gamepad(const gamepad &other) :
  buttons(other.buttons),
  analogs(other.analogs),
  deadzone(other.deadzone)
{

}

ssbml::gamepad::gamepad(const compressed &c) :
  deadzone(DEFAULT_DEAD_ZONE)
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
