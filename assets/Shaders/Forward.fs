#version 300 es
// BoydEngine - Standard fragment shader - Forward rendering stage
precision highp float;

uniform sampler2D u_DiffuseMap;

in vec3 vo_Normal;
in vec4 vo_Tint;
in vec2 vo_TexCoord;

out vec4 fo_FragColor;

void main()
{
    vec4 diffuse = texture(u_DiffuseMap, vo_TexCoord).rgba;
    fo_FragColor = diffuse * vo_Tint;
}
