#version 300 es
// BoydEngine - Standard fragment shader - Skybox
precision highp float;

uniform samplerCube u_EnvMap;

in vec3 vo_CubeCoord;

out vec4 fo_FragColor;

void main()
{
    fo_FragColor = texture(u_EnvMap, vo_CubeCoord).rgba;
}
