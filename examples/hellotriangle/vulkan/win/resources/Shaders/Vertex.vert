#version 450

layout (location = 0) in vec3 PosL;
layout (location = 1) in vec4 Color;

layout (location = 0) out vec4 v_Color;

void main()
{
    gl_Position = vec4(PosL, 1.0);
    v_Color = Color;
}