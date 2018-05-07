#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstring>

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ssbml
{
  class shader
  {
  public:
    void use();
    GLint get_uniform_location(std::string name);
    static void set_uniform(GLint location, glm::mat4 &matrix);
    static void set_uniform(GLint location, float v0, float v1, float v2);
    static void set_uniform(GLint location, int i);

    shader(std::string vertexPath, std::string fragmentPath);
    ~shader();

  private:
    GLint program;
  };
}

#endif
