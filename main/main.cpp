/*! \file main.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_config.h"
#include "cg_logger.h"

#include "app.h"

int main(int /*argc*/, char ** /*argv*/) {
    CG::Logger::getInstance().configure("data/logger.cfg", "Logger");

    try {
        App::Parameters params;
        params.windowSizeX = 1200;
        params.windowSizeY = 800;
        params.windowTitle = "Data Transfer Over Sound";

        App app(params);

        while (true) {
            app.update();
            app.render();

            if (app.shouldTerminate()) break;
        }

        app.terminate();
    } catch (CG::Exception &ex) {
        CG_FATAL(0, "%s\n", ex._info);
    }

    return 0;
}
