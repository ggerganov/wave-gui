/*! \file cg_mouse.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_mouse.h"

namespace CG {

Mouse::Mouse() {
    _x = _y = _dx = _dy = 0.0;
}

Mouse::~Mouse() {
}

void Mouse::update(double xpos, double ypos) {
    std::lock_guard<std::mutex> lock(_mutex);

    _dx = xpos - _x;
    _dy = ypos - _y;
    _x = xpos;
    _y = ypos;
}

void Mouse::update(int button, int action, int /*mods*/) {
    std::lock_guard<std::mutex> lock(_mutex);

    _buttons[button] = action;
}

}
