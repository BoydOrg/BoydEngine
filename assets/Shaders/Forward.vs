#version 300 es
// BoydEngine - Standard vertex shader - Forward rendering stage
precision mediump float;

uniform mat4 u_ModelViewProjection;

layout(location = 0) in vec3 vi_Position;
layout(location = 1) in vec3 vi_Normal;
layout(location = 2) in vec4 vi_Tint;
layout(location = 3) in vec2 vi_TexCoord;

out vec3 vo_Normal;
out vec4 vo_Tint;
out vec2 vo_TexCoord;

void main()
{
    vo_Normal = vi_Normal;
    vo_Tint = vi_Tint;
    vo_TexCoord = vi_TexCoord;
    gl_Position = u_ModelViewProjection * vec4(vi_Position, 1.0);
}
