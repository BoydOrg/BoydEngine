#version 300 es
precision mediump float;

// BoydEngine - Standard fragment shader

uniform sampler2D u_DiffuseMap;

in vec3 vo_Normal;
in vec4 vo_TintEmission;
in vec2 vo_TexCoord;

out vec4 fo_FragColor;

void main()
{
    vec4 diffuse = texture(u_DiffuseMap, vo_TexCoord).rgba;
    fo_FragColor = vec4(diffuse.rgb * vo_TintEmission.rgb, diffuse.a);
}