#version 330 core
layout (location = 0) in vec2 inPos;
void main()
{
    gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
}