#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main()
{
    vec4 color = texture(texture0, fragTexCoord);

    float glowFactor = 1.5;
    color.rgb *= glowFactor;

    finalColor = color * colDiffuse;
}
