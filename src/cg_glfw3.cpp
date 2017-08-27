/*! \file cg_glfw3.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_glfw3.h"

#include "cg_logger.h"

#include <GLFW/glfw3.h>

#include <mutex>

namespace {
void error_callback(int error, const char * description) {
    CG_FATAL(0, "GLFW Error %d: %s\n", error, description);
}
}

namespace CG {

struct GLFW3::Data {
    mutable std::mutex mutex;

    int initCalls = 0;
};

GLFW3::GLFW3() : _data(new GLFW3::Data()) {}

GLFW3::~GLFW3() {
    std::lock_guard<std::mutex> lock(_data->mutex);
    if (_data->initCalls > 0) {
        glfwTerminate();
    }
}

bool GLFW3::init() {
    std::lock_guard<std::mutex> lock(_data->mutex);

    if (_data->initCalls == 0) {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            return false;
        }
    }

    ++_data->initCalls;
    return true;
}

bool GLFW3::poll() const {
    glfwPollEvents();

    return true;
}

bool GLFW3::terminate() {
    std::lock_guard<std::mutex> lock(_data->mutex);

    if (_data->initCalls == 1) {
        glfwTerminate();
    }

    --_data->initCalls;
    return true;
}

}
