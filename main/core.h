/*! \file core.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <memory>

namespace Data {
struct StateData;
struct StateInput;
}

class Core {
public:
    Core();
    ~Core();

    void init();
    void update();
    void terminate();

    std::weak_ptr<::Data::StateData> getStateData() const;
    void setStateInput(std::weak_ptr<::Data::StateInput> stateInput);

    enum Event {
        Init,
        DataOn,
        DataOff,
        DataTap,
        DataSend,
        DataClear,
    };

    void addEvent(Event event);

private:
    void input();
    void main();
    void cache();

    struct Data;
    std::unique_ptr<Data> _data;
};
