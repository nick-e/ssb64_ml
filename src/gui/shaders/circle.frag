#version 410 core

out vec4 colorOut;

uniform vec3 color;

void main()
{
  float r = 0.0, delta = 0.0, alpha = 1.0;
  vec2 cxy = 2.0 * gl_PointCoord - 1.0;
  r = dot(cxy, cxy);
  delta = fwidth(r) * 0.7;
  alpha = 1.0 - smoothstep(1.0 - delta, 1.0 + delta, r);

  colorOut = vec4(color, alpha);
}
