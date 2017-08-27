/*! \file cg_mouse.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <mutex>
#include <map>

namespace CG {

class Mouse {
private:
    Mouse();
    ~Mouse();

public:
    inline static Mouse & getInstance() {
        static Mouse instance;
        return instance;
    }

    typedef int TypeButton;
    typedef int TypeAction;

    void update(double xpos, double ypos);
    void update(int button, int action, int mods);

    double _x;
    double _y;
    double _dx;
    double _dy;

    std::map<TypeButton, TypeAction> _buttons;

private:
    mutable std::mutex _mutex;
};

}
