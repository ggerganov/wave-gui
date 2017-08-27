/*! \file cg_window2d.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include "cg_window.h"

#include <map>

namespace CG {
class Window2D : public Window {
public:
    Window2D();
    ~Window2D();

    virtual void render() const override;
    virtual void updateWindowSize() override;

    virtual float getViewportSizeX() const;
    virtual float getViewportSizeY() const;
    virtual void setViewport(int vid, float x0, float y0, float dx, float dy);
    virtual void applyViewport(int vid, bool clip = true);
    virtual void resetViewport() const;

    virtual void toViewportCordinates(int vid, float & x, float & y) const;

    float getRatioXY() const { return _ratioXY; }

private:
    virtual void setupWindowParameters() override;

    float _ratioXY;
    float _fXmin;
    float _fXmax;

    int _currentViewportId;
    struct Viewport;
    std::map<int, Viewport> _viewports;
};
}
