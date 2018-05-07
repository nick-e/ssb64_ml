#version 410 core

uniform mat4 model;
uniform mat4 projection;

void main()
{
  gl_Position = projection * model * vec4(0.0, 0.0, 0.0, 1.0);
}
