#version 300 es
// BoydEngine - Standard vertex shader - Skybox
precision mediump float;

uniform mat4 u_ModelViewProjection;

layout(location = 0) in vec3 vi_Position;
layout(location = 1) in vec3 vi_Normal;
layout(location = 2) in vec4 vi_Tint;
layout(location = 3) in vec2 vi_TexCoord;

out vec3 vo_CubeCoord;

void main()
{
    vo_CubeCoord = vi_Position;
    vec4 ndcPos = u_ModelViewProjection * vec4(vi_Position, 1.0);
    gl_Position = ndcPos.xyww;
}
