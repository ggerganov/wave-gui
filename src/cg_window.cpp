/*! \file cg_window.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_window.h"

#include "cg_logger.h"
#include "cg_config.h"
#include "cg_mouse.h"
#include "cg_keyboard.h"

#include <GLFW/glfw3.h>

namespace CG {

namespace WindowStatic {
static std::map<GLFWwindow *, Window*> g_windowMap;

static void key_callback(GLFWwindow* /*window*/, int key, int scancode, int action, int mods) {
    static Keyboard & k = Keyboard::getInstance();

    k.update(key, scancode, action, mods);
}

static void cursor_position_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
    static Mouse & m = Mouse::getInstance();

    m.update(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int mods) {
    static Mouse & m = Mouse::getInstance();

    m.update(button, action, mods);
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
    g_windowMap[window]->changeWindowSize(width, height);
}
}

Window::Window() {
    _window = NULL;

    _windowWidth  = -1;
    _windowHeight = -1;

    _framebufferWidth  = -1;
    _framebufferHeight = -1;

    _ratioFWWidth = 1.0f;
    _ratioFWHeight = 1.0f;

    _isResizable = false;
    _isFullscreen = false;

    _wasSizeChanged = false;
}

Window::~Window() {
    if (_window) {
        WindowStatic::g_windowMap[_window] = NULL;
        glfwDestroyWindow(_window);
    }
}

void Window::initialize(int width, int height, const char *title, bool isResizable) {
    _windowWidth = width;
    _windowHeight = height;

    _isResizable = isResizable;
    _isFullscreen = false;

    glfwWindowHint(GLFW_RESIZABLE, isResizable);
    _window = glfwCreateWindow(_windowWidth, _windowHeight, title, NULL, NULL);

    if (!_window) {
        throw Exception("[GLFW] Unable to create window.\n");
    }
    WindowStatic::g_windowMap[_window] = this;

    setupWindowParameters();
}

void Window::initializeWindowedFullscreen(const char *title) {
    _isResizable = false;
    _isFullscreen = true;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    _windowWidth = mode->width;
    _windowHeight = mode->height;

    _window = glfwCreateWindow(_windowWidth, _windowHeight, title, glfwGetPrimaryMonitor(), NULL);
    if (!_window) {
        throw Exception("[GLFW] Unable to create window.\n");
    }
    WindowStatic::g_windowMap[_window] = this;

    setupWindowParameters();
}

void Window::setClearColor(float r, float g, float b, float a) const {
    glClearColor(r, g, b, a);
}

void Window::setSwapInterval(int interval) const {
    glfwSwapInterval(interval);
}

void Window::makeContextActive() const {
    if (_window) glfwMakeContextCurrent(_window);
    else throw Exception("[GLFW] Unable to make context active, since window is not initialized.\n");
}

void Window::swapBuffers() const {
    if (_window) glfwSwapBuffers(_window);
    else throw Exception("[GLFW] Unable to swap buffers, since window is not initialized.\n");
}

void Window::render() const {
    glViewport(0, 0, _framebufferWidth, _framebufferHeight);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Window::changeWindowSize(int width, int height) {
    _wasSizeChanged = true;

    //glfwGetWindowSize(_window, &_windowWidth, &_windowHeight);
    _windowWidth = width;
    _windowHeight = height;
}

void Window::updateWindowSize() {
    glfwGetFramebufferSize(_window, &_framebufferWidth, &_framebufferHeight);

    _ratioFWWidth = _framebufferWidth/_windowWidth;
    _ratioFWHeight = _framebufferHeight/_windowHeight;

    _wasSizeChanged = false;
}

void Window::toScreenCoordinates(float &x, float &y) {
    x = 2.0*(x/_windowWidth  - 0.5);
    y = 2.0*(y/_windowHeight - 0.5);
}

void Window::setupWindowParameters() {
    glfwMakeContextCurrent(_window);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(1);
    glfwSetKeyCallback(_window, WindowStatic::key_callback);
    glfwSetCursorPosCallback(_window, WindowStatic::cursor_position_callback);
    glfwSetMouseButtonCallback(_window, WindowStatic::mouse_button_callback);
    glfwSetWindowSizeCallback(_window, WindowStatic::window_size_callback);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    changeWindowSize(_windowWidth, _windowHeight);
    updateWindowSize();

    CG_DBGD(5, "[GLFW] Initialized new window:\n\
        Window      size: %d x %d \n\
        Framebuffer size: %d x %d \n\
        Resizable       : %s \n\
        Fullscreen      : %s \n",
            _windowWidth, _windowHeight,
            _framebufferWidth, _framebufferHeight,
            (_isResizable) ? "yes" : "no",
            (_isFullscreen) ? "yes" : "no");
    CG_DBGD(5, "\n");
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(_window);
}

}
