#version 410
in vec3 ciPosition;

uniform mat4 ciModelViewProjection;

void main(void)
{
  vec4 pos = vec4(ciPosition, 1.0);
  //pos.x = pos.x * 100.0;
  //pos.z = pos.z * 100.0;
  gl_Position = ciModelViewProjection * pos;
}
