#version 450 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;
layout(location = 2) in float opacity;
layout(location = 3) in float psize;

out vec3 fragColor;
out float alpha;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * position;
    gl_PointSize = 3.0;
    fragColor = color;
    alpha = opacity;
};
