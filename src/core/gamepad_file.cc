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
    throw std::runtime_error("Failed to read gamepad file");
  }
  decompress(c);
}

ssbml::gamepad_file& ssbml::gamepad_file::operator>>(compressed &c)
{
  get_next2(c);
  return *this;
}

ssbml::gamepad_file& ssbml::gamepad_file::operator>>(uint8_t *buf)
{
  get_next2(*((compressed*)(buf)));
  return *this;
}

void ssbml::gamepad_file::get_next2(compressed &c)
{
  if (!stream.read((char*)&c, sizeof(c)))
  {
    throw std::runtime_error("Failed to read gamepad file");
  }
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
