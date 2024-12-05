#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

const float offset = 1.0 / 300.0;

void main()
{
    vec4 result = vec4(0.0);

    // Muestra de píxeles adyacentes
    result += texture(texture0, fragTexCoord + vec2(-offset, -offset)) * 0.0625;
    result += texture(texture0, fragTexCoord + vec2( 0.0,    -offset)) * 0.125;
    result += texture(texture0, fragTexCoord + vec2( offset, -offset)) * 0.0625;
    result += texture(texture0, fragTexCoord + vec2(-offset,  0.0   )) * 0.125;
    result += texture(texture0, fragTexCoord)                          * 0.25;
    result += texture(texture0, fragTexCoord + vec2( offset,  0.0   )) * 0.125;
    result += texture(texture0, fragTexCoord + vec2(-offset,  offset)) * 0.0625;
    result += texture(texture0, fragTexCoord + vec2( 0.0,     offset)) * 0.125;
    result += texture(texture0, fragTexCoord + vec2( offset,  offset)) * 0.0625;

    finalColor = result;
}
