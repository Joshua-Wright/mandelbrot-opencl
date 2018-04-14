//
// Created by j0sh on 4/14/18.
//

// necessary preprocessor definitions to make other headers work

#pragma once

#ifdef OS_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif

#ifdef OS_LNX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#define CL_HPP_ENABLE_EXCEPTIONS
