// Vertex Shader (basic passthrough)
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 fragTexCoord;

void main()
{
    fragTexCoord = vertexTexCoord;
    gl_Position = vec4(vertexPosition, 1.0);
}

// Fragment Shader (gravitational lensing effect)
#version 330

in vec2 fragTexCoord;

out vec4 finalColor;

uniform sampler2D texture0;  // The texture to distort (e.g., the background)
uniform vec2 blackHoleCenter;  // The center of the black hole (in screen coordinates, normalized 0-1)
uniform float blackHoleRadius; // The radius of the event horizon
uniform float distortionStrength; // How much the light is bent

void main()
{
    // Calculate distance from the black hole center
    vec2 uv = fragTexCoord;
    vec2 toCenter = uv - blackHoleCenter;
    float dist = length(toCenter);

    // Distortion factor (gravitational lensing effect)
    if (dist < blackHoleRadius) {
        // Inside event horizon, show black (nothing escapes)
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float lensEffect = distortionStrength / (dist * dist);

    // Apply distortion: shift UV coordinates
    vec2 distortedUV = uv + normalize(toCenter) * lensEffect;

    // Sample the texture with distorted UV
    vec4 color = texture(texture0, distortedUV);

    // Blend with an optional glow or falloff
    float glow = smoothstep(blackHoleRadius, blackHoleRadius * 1.5, dist);
    finalColor = mix(vec4(0.0, 0.0, 0.0, 1.0), color, glow);
}
