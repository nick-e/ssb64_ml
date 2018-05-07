#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int ssbml::texture::get_width()
{
  return width;
}

int ssbml::texture::get_height()
{
  return height;
}

void ssbml::texture::bind()
{
  glBindTexture(GL_TEXTURE_2D, _texture);
  check_gl_error();
}

ssbml::texture::texture(std::string source)
{
  glGenTextures(1, &_texture);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, _texture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  check_gl_error();

  unsigned char *data = stbi_load(source.c_str(), &width, &height,
    &numChannels, 0);
  if (data == NULL)
  {
    throw std::runtime_error("\"" + source + "\": Could not load image");
  }
  if (numChannels == 3)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
      GL_UNSIGNED_BYTE, data);
  }
  else if (numChannels == 4)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
      GL_UNSIGNED_BYTE, data);
  }
  else
  {
    throw std::runtime_error("\"" + source + "\" contains "
      + std::to_string(numChannels) + " which is irregular");
  }
  check_gl_error();
  glGenerateMipmap(GL_TEXTURE_2D);
  check_gl_error();

  stbi_image_free(data);
}

ssbml::texture::~texture()
{
  glDeleteTextures(1, &_texture);
}
