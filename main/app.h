/*! \file app.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <memory>
#include <string>

namespace CG {
class Window2D;
}

class UI;
class Core;

class App {
public:
    struct Parameters {
        int windowSizeX = 1440;
        int windowSizeY = 1200;

        std::string windowTitle = "Noname App";
    };

    explicit App(const Parameters & params);
    ~App();

    void init();
    void terminate();

    void update();
    void render() const;

    inline bool shouldTerminate() const { return _shouldTerminate; }

    bool _shouldTerminate = false;

private:
    std::shared_ptr<UI> _ui;
    std::shared_ptr<Core> _core;
    std::shared_ptr<CG::Window2D> _window;
};
