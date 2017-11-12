#version 410
in vec2 ciPosition;

uniform mat4 ciModelViewProjection;

void main(void)
{
  vec4 pos = vec4(ciPosition.x * 1000.0, 0.0, ciPosition.y * 1000.0, 1.0);
  gl_Position = ciModelViewProjection * pos;
}
