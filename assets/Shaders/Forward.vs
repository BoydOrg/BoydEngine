#version 300 es
precision mediump float;
// BoydEngine - Standard vertex shader - Forward rendering stage

uniform mat4 u_ModelViewProjection;

layout(location = 0) in vec3 vi_Position;
layout(location = 1) in vec3 vi_Normal;
layout(location = 2) in vec4 vi_TintEmission;
layout(location = 3) in vec2 vi_TexCoord;

out vec3 vo_Normal;
out vec4 vo_TintEmission;
out vec2 vo_TexCoord;

void main()
{
    vo_Normal = vi_Normal;
    vo_TintEmission = vi_TintEmission;
    vo_TexCoord = vi_TexCoord;
    gl_Position = u_ModelViewProjection * vec4(vi_Position, 1.0);
}