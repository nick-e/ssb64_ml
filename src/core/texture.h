#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <iostream>
#include <stdexcept>

#include <GL/glew.h>
#include <GL/gl.h>

#include "util.h"

namespace ssbml
{
  class texture
  {
  public:
    int get_width();
    int get_height();
    void bind();

    texture(std::string source);
    ~texture();

  private:
    int height;
    int numChannels;
    int width;
    GLuint _texture;
  };
}

#endif
