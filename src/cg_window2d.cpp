/*! \file cg_window2d.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_window2d.h"

#include "GLFW/glfw3.h"

namespace CG {

struct Window2D::Viewport { float x0, y0, dx, dy; };

Window2D::Window2D() {
}

Window2D::~Window2D() {
}

void Window2D::render() const {
    //glViewport(0, 0, _framebufferWidth, _framebufferHeight);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.f, 1.f, 1.f, -1.f, 1.f, -1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Window2D::updateWindowSize() {
    Window::updateWindowSize();

    _ratioXY = _framebufferWidth / (float) _framebufferHeight;
    _fXmin   = -_ratioXY;
    _fXmax   =  _ratioXY;
}

float Window2D::getViewportSizeX() const {
    try {
        return _viewports.at(_currentViewportId).dx;
    } catch (...) {
    }
    return 0.0f;
}

float Window2D::getViewportSizeY() const {
    try {
        return _viewports.at(_currentViewportId).dy;
    } catch (...) {
    }
    return 0.0f;
}

void Window2D::setViewport(int vid, float x0, float y0, float dx, float dy) {
    _viewports[vid].x0 = x0;
    _viewports[vid].y0 = _windowHeight - y0;
    _viewports[vid].dx = dx;
    _viewports[vid].dy = -dy;
}

void Window2D::applyViewport(int vid, bool clip) {
    try {
        auto & viewport = _viewports.at(vid);

        float dx = (2.0f*_windowWidth)/viewport.dx;
        float dy = (2.0f*_windowHeight)/viewport.dy;
        float x0 = -1.0f - (dx*viewport.x0)/_windowWidth;
        float y0 = -1.0f - (dy*viewport.y0)/_windowHeight;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(x0, x0 + dx, y0, y0 + dy, 1.f, -1.f);

        if (clip) {
            glScissor(
                    _ratioFWWidth*viewport.x0,
                    _ratioFWHeight*viewport.y0 + _ratioFWHeight*viewport.dy,
                    _ratioFWWidth*viewport.dx,
                    -_ratioFWHeight*viewport.dy);
            glEnable(GL_SCISSOR_TEST);
        }

        _currentViewportId = vid;
    } catch (...) {
    }
}

void Window2D::resetViewport() const {
    glDisable(GL_SCISSOR_TEST);
}

void Window2D::toViewportCordinates(int vid, float & x, float & y) const {
    try {
        auto & viewport = _viewports.at(vid);

        y = _windowHeight - y;
        x = 2.0f*((x - viewport.x0)/viewport.dx) - 1.0f;
        y = 2.0f*((y - viewport.y0)/viewport.dy) - 1.0f;
    } catch (...) {
        x = y = 0.0f;
    }
}

void Window2D::setupWindowParameters() {
    Window::setupWindowParameters();

    setViewport(0, 0.f, _windowHeight, _windowWidth, _windowHeight);
}

}
