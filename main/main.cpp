/*! \file main.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#ifdef __EMSCRIPTEN__
#include "emscripten/emscripten.h"
#endif

#include "cg_config.h"
#include "cg_logger.h"

#include "app.h"

#include <functional>

static std::function<bool()> g_update;

void update() {
    g_update();
}

int main(int /*argc*/, char ** argv) {
    CG::Logger::getInstance().configure("data/logger.cfg", "Logger");

    CG_INFO(0, "Capture device name: %s\n", argv[1]);

    App::Parameters params;
    params.windowSizeX = 1200;
    params.windowSizeY = 800;
    params.windowTitle = "Data Transfer Over Sound";

    App app(params);

    g_update = [&]() {
        app.update();
        app.render();

        if (app.shouldTerminate()) return false;
        return true;
    };

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 60, 1);
#else
    while (true) {
        if (g_update() == false) break;
    }
#endif

    app.terminate();

    return 0;
}
