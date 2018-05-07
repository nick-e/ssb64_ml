#include "gamepad_file.h"

void ssbml::gamepad_file::rewind()
{
  stream.seekg(-sizeof(compressed), std::ios_base::cur);
}

void ssbml::gamepad_file::get_next()
{
  compressed c;
  if (!stream.read((char*)&c, sizeof(c)))
  {
    throw std::runtime_error(std::string("Failed to read gamepad file: ")
      + strerror(errno));
  }
  update(c);
}

void ssbml::gamepad_file::get_next(compressed &c)
{
  if (!stream.read((char*)&c, sizeof(c)))
  {
    throw std::runtime_error(std::string("Failed to read gamepad file: ")
      + strerror(errno));
  }
  update(c);
}

ssbml::gamepad_file& ssbml::gamepad_file::operator>>(compressed &c)
{
  get_next(c);
  return *this;
}

ssbml::gamepad_file& ssbml::gamepad_file::operator>>(uint8_t *buf)
{
  get_next(*((compressed*)(buf)));
  return *this;
}

ssbml::gamepad_file::gamepad_file(std::string fileName) :
  stream(fileName, std::ios::binary)
{
  if (!stream)
  {
    throw std::runtime_error("Failed to open " + fileName);
  }
}

ssbml::gamepad_file::~gamepad_file()
{
  stream.close();
}
