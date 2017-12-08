#version 410

uniform vec4 lineColor;

layout(location = 0) out vec4 out_color;

void main(void)
{
  out_color = vec4(lineColor.xyz, 1.0);
}
