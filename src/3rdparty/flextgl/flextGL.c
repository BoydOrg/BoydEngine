/*
    This file was generated using https://github.com/mosra/flextgl:

        path/to/flextGLgen.py -D generated -t templates/glfw3-es profiles/gles30.txt

    Do not edit directly, modify the template or profile and regenerate.
*/

#include "flextGL.h"

#include <stdio.h>
#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif

void flextLoadOpenGLFunctions(void) {
}

int flextInit(GLFWwindow* window) {
    int major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    int minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);

    flextLoadOpenGLFunctions();

    /* Check for minimal version */

    if(major * 10 + minor < 30) {
        fprintf(stderr, "Error: OpenGL version 3.0 not supported.\n");
        fprintf(stderr, "       Your version is %d.%d.\n", major, minor);
        fprintf(stderr, "       Try updating your graphics driver.\n");
        return GL_FALSE;
    }

    /* Check for extensions */

    if(glfwExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
        FLEXT_EXT_texture_filter_anisotropic = GL_TRUE;
    }

    if(glfwExtensionSupported("GL_EXT_texture_compression_s3tc")) {
        FLEXT_EXT_texture_compression_s3tc = GL_TRUE;
    }

    return GL_TRUE;
}

/* Extension flag definitions */
int FLEXT_EXT_texture_filter_anisotropic = GL_FALSE;
int FLEXT_EXT_texture_compression_s3tc = GL_FALSE;

/* Function pointer definitions */

#ifdef __cplusplus
}
#endif
