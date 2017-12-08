#version 410
in vec3 ciPosition;

uniform mat4 ciModelViewProjection;

void main(void)
{
  vec4 pos = vec4(1000.0 * ciPosition, 1.0);
  gl_Position = ciModelViewProjection * pos;
}
