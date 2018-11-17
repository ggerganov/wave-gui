/*! \file app.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "app.h"

#include "cg_config.h"
#include "cg_logger.h"
#include "cg_window2d.h"

#include "ui.h"
#include "core.h"

namespace {
}

App::App(const Parameters & params) {
    CG_INFO(0, "Creating App object\n");

    _window = std::make_shared<CG::Window2D>();
    _core = std::make_shared<Core>();
    _ui = std::make_shared<UI>();

    _window->initialize(params.windowSizeX, params.windowSizeY, params.windowTitle.c_str(), true);
    _window->makeContextActive();
    _window->setSwapInterval(1);

    _ui->setStateData(_core->getStateData());
    _core->setStateInput(_ui->getStateInput());

    _ui->setEventCallback(UI::KEY_ESCAPE, [this]() { _shouldTerminate = true; });
    _ui->setEventCallback(UI::BUTTON_INIT, [this]() { _core->addEvent(Core::Init); });
    _ui->setEventCallback(UI::BUTTON_DATA_ON, [this]() { _core->addEvent(Core::DataOn); });
    _ui->setEventCallback(UI::BUTTON_DATA_OFF, [this]() { _core->addEvent(Core::DataOff); });
    _ui->setEventCallback(UI::BUTTON_DATA_TAP, [this]() { _core->addEvent(Core::DataTap); });
    _ui->setEventCallback(UI::BUTTON_DATA_SEND, [this]() { _core->addEvent(Core::DataSend); });
    _ui->setEventCallback(UI::BUTTON_DATA_CLEAR, [this]() { _core->addEvent(Core::DataClear); });

    _ui->init(_window);
    _core->init();
}

App::~App() {
    CG_INFO(0, "Destroying App object\n");

    _window.reset();
    _ui.reset();
    _core.reset();
}


void App::terminate() {
    CG_INFO(0, "Terminating application\n");

    _core->terminate();
    _ui->terminate();
}

void App::update() {
    if (_window->shouldClose()) {
        _shouldTerminate = true;
    }

    _window->makeContextActive();
    if (_window->wasWindowSizeChanged()) {
        _window->updateWindowSize();
    }

    _ui->update();
    _core->update();
}

void App::render() const {
    _window->render();
    _ui->render();

    _window->swapBuffers();
}
