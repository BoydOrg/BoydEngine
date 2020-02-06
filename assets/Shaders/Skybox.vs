/*
   Skybox vertex shader.
 */

#version 330

in vec3 vertexPosition;

uniform mat4 projection;
uniform mat4 view;

out vec3 fragPosition;

void main()
{
    // Keep the old position in model space
    fragPosition = vertexPosition;

    // And get the view-space position.
    // Since translating a cubemap makes no sense so we zero out the fourth column
    mat4 rotView = mat4(mat3(view));
    vec4 clipPosition = projection * rotView * vec4(vertexPosition, 1.0);

    gl_Position = clipPosition.xyzw;
}