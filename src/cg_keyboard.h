/*! \file cg_keyboard.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <mutex>
#include <memory>
#include <map>

namespace CG {
class Timer;

class Keyboard {
private:
    Keyboard();
    ~Keyboard();

public:
    inline static Keyboard & getInstance() {
        static Keyboard instance;
        return instance;
    }

    enum TypeState { KEY_NOT_PRESSED, KEY_PRESSED };

    typedef int TypeKey;
    typedef int TypeAction;

private:
    typedef struct {
        float lastUpdated;
        TypeState state;
    } TypeKeyState;

public:
    void update(TypeKey key, int scancode, TypeAction action, int mods);

    bool isPressed(TypeKey key);
    TypeState getState(TypeKey key);

    std::map<TypeKey, TypeAction>   _actions;
    std::map<TypeKey, TypeKeyState> _keys;

private:
    mutable std::mutex _mutex;

    std::unique_ptr<Timer> _timer;
};

}
