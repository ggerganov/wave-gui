/*! \file cg_keyboard.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_keyboard.h"
#include "cg_timer.h"

#define KEY_TIMEOUT 60

namespace CG {

Keyboard::Keyboard() {
    _timer.reset(new Timer());
}

Keyboard::~Keyboard() {
}


void Keyboard::update(int key, int /*scancode*/, int action, int /*mods*/) {
    _mutex.lock();
    _actions[key] = action;
    if (action > 0) {
        _keys[key].state = KEY_PRESSED;
        _keys[key].lastUpdated = _timer->time();
    } else {
        _keys[key].state = KEY_NOT_PRESSED;
    }
    _mutex.unlock();
}

bool Keyboard::isPressed(Keyboard::TypeKey key) {
    if (_keys[key].state == Keyboard::KEY_NOT_PRESSED) return false;
    return (_timer->time() - _keys[key].lastUpdated > KEY_TIMEOUT) ?
           false : true;
}

Keyboard::TypeState Keyboard::getState(Keyboard::TypeKey key) {
    if (_keys[key].state == Keyboard::KEY_NOT_PRESSED)
        return Keyboard::KEY_NOT_PRESSED;
    return (_timer->time() - _keys[key].lastUpdated > KEY_TIMEOUT) ?
           Keyboard::KEY_NOT_PRESSED : Keyboard::KEY_PRESSED;
}

}
