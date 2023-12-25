#version 450 core

layout(location = 0) out vec4 color;
in vec3 fragColor;
in float alpha;

void main()
{
    color = vec4(fragColor, alpha);
};
