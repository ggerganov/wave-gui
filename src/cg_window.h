/*! \file cg_window.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

struct GLFWwindow;

namespace CG {

class Window {
public:
    Window();
    virtual ~Window();

    virtual void initialize(int width, int height, const char *title, bool isResizable = true);
    virtual void initializeWindowedFullscreen(const char *title);

    virtual void setClearColor(float r, float g, float b, float a) const;
    virtual void setSwapInterval(int interval) const;
    virtual void makeContextActive() const;
    virtual void swapBuffers() const;

    virtual void render() const;

    virtual void changeWindowSize(int width, int height);
    virtual bool wasWindowSizeChanged() const { return _wasSizeChanged; }
    virtual void updateWindowSize();

    virtual void toScreenCoordinates(float &x, float &y);

    virtual GLFWwindow * getGLFWWindow() const { return _window; }

    virtual bool shouldClose() const;

protected:
    virtual void setupWindowParameters();

    GLFWwindow *_window;

    int _windowWidth;
    int _windowHeight;

    int _framebufferWidth;
    int _framebufferHeight;

    // ratio of frame size to window size
    float _ratioFWWidth;
    float _ratioFWHeight;

    bool _isResizable;
    bool _isFullscreen;

    bool _wasSizeChanged;

private:
};

}
