#include "shader.h"

void ssbml::shader::use()
{
  glUseProgram(program);
}

GLint ssbml::shader::get_uniform_location(std::string name)
{
  return glGetUniformLocation(program, name.c_str());
}

void ssbml::shader::set_uniform(GLint location, glm::mat4 &matrix)
{
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ssbml::shader::set_uniform(GLint location, float v0, float v1, float v2)
{
  glUniform3f(location, v0, v1, v2);
}

void ssbml::shader::set_uniform(GLint location, int i)
{
  glUniform1i(location, i);
}

ssbml::shader::shader(std::string vertexPath, std::string fragmentPath) :
  program(glCreateProgram())
{
  const char *src;
  char buf[2048];
  int success;
  GLint vertex = glCreateShader(GL_VERTEX_SHADER);
  GLint fragment = glCreateShader(GL_FRAGMENT_SHADER);
  std::ifstream fragmentFile;
  std::ifstream vertexFile;
  std::string fragmentSource;
  std::string vertexSource;

  vertexFile.open(vertexPath);
  if (vertexFile.fail())
  {
    throw std::runtime_error(vertexPath + ": " + strerror(errno));
  }
  vertexSource.assign((std::istreambuf_iterator<char>(vertexFile)),
    (std::istreambuf_iterator<char>()));
  vertexFile.close();
  src = vertexSource.c_str();
  glShaderSource(vertex, 1, &src, NULL);
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertex, sizeof(buf), NULL, buf);
    throw std::runtime_error(std::string("Vertex shader compile error: ")
      + buf);
  }

  fragmentFile.open(fragmentPath);
  if (fragmentFile.fail())
  {
    throw std::runtime_error(fragmentPath + ": " + strerror(errno));
  }
  fragmentSource.assign((std::istreambuf_iterator<char>(fragmentFile)),
    (std::istreambuf_iterator<char>()));
  fragmentFile.close();
  src = fragmentSource.c_str();
  glShaderSource(fragment, 1, &src, NULL);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragment, sizeof(buf), NULL, buf);
    throw std::runtime_error(std::string("Fragment shader compile error: ")
      + buf);
  }

  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(program, sizeof(buf), NULL, buf);
    throw std::runtime_error(std::string("Shader program link error: ")
      + buf);
  }
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

ssbml::shader::~shader()
{

}
