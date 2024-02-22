#version 330 core
in vec3 fragColor;
in vec2 fragTexCoord;

out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, fragTexCoord) * vec4(fragColor, 1.0);
}
