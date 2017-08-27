/*! \file ui.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <memory>
#include <functional>

namespace CG {
class Window2D;
}

namespace Data {
struct StateData;
struct StateInput;
}

class UI {
public:
    UI();
    ~UI();

    bool init(std::weak_ptr<CG::Window2D> window);
    void update();
    void render() const;
    void terminate();

    void setStateData(std::weak_ptr<::Data::StateData> stateData);
    std::weak_ptr<::Data::StateInput> getStateInput() const;

    enum Event {
        KEY_ESCAPE,
        BUTTON_INIT,
        BUTTON_DATA_ON,
        BUTTON_DATA_OFF,
        BUTTON_DATA_TAP,
        BUTTON_DATA_SEND,
        BUTTON_DATA_CLEAR,
    };

    void setEventCallback(Event e, std::function<void()> && callback);

private:
    void pollEvents();
    void processKeyboard();

    void renderMainMenuBar() const;
    void renderWindowControls() const;
    void renderWindowInput() const;
    void renderWindowOutput() const;

    struct Data;
    std::unique_ptr<Data> _data;
};
