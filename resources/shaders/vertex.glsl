#version 450 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
layout(location = 2) in float opacity;
layout(location = 3) in float psize;

out vec4 fragColor;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * position;
    fragColor = vec4(color, opacity);
};
