#version 330 core
layout (location = 0) in vec2 inPos;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    vec4 position = uP * uV * uM * vec4(inPos, 0.0, 1.0);
    gl_Position = position;
}
