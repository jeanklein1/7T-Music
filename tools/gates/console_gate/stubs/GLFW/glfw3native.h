// STUB — declarations only, the native boot remains the runtime witness.
//
// The expose getters console.hpp names, each behind the same expose macro
// real glfw3native.h gates it with. Return types are OPAQUE on purpose:
// upstream's header pulls <X11/Xlib.h> (and Xrandr) under
// GLFW_EXPOSE_NATIVE_X11 and <windows.h> under _WIN32, and a gate that
// needed a platform SDK would run on one machine instead of anywhere.
// void* and an unsigned long convert to what the surface sources hold —
// SurfaceSourceXlibWindow::display is void*, ::window is uint64_t;
// SurfaceSourceWindowsHWND::hwnd and ::hinstance are both void*.
//
// The ORDER matters and console.hpp already honours it: <GLFW/glfw3.h>
// first, then the expose macros, then this header — upstream requires
// exactly that, and so does the GLFWwindow* in these signatures.
#pragma once

#if defined(GLFW_EXPOSE_NATIVE_WIN32)
void* glfwGetWin32Window(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_X11)
void* glfwGetX11Display(void);
unsigned long glfwGetX11Window(GLFWwindow* window);
#endif

#if defined(GLFW_EXPOSE_NATIVE_COCOA)
void* glfwGetCocoaWindow(GLFWwindow* window);
#endif
