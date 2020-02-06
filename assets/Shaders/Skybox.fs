/*
   Skybox fragment shader.
 */

#version 330

in vec3 fragPosition;

uniform samplerCube environmentMap;

out vec4 finalColor;

void main()
{
    vec3 color = texture(environmentMap, fragPosition).rgb;

    // Perform some gamma correction
    float gamma = 2.2;

    color = color/(color + vec3(1.0));
    color = pow(color, vec3(1.0/gamma));
    finalColor = vec4(color, 1.0);
}