#version 330 core
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

out vec3 fragColor;
out vec2 fragTexCoord;

uniform mat4 um4p;
uniform mat4 um4v;
uniform mat4 um4m;

void main()
{
    gl_Position = um4p * um4v * um4m * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}