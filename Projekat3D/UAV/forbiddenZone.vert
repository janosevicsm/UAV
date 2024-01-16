#version 330 core
layout (location = 0) in vec2 inPos;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    gl_Position = uP * uV * uM * vec4(inPos.x, 0.0001, -inPos.y, 1.0);
}