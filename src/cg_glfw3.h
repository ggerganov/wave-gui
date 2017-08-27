/*! \file cg_glfw3.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <memory>

namespace CG {

class GLFW3 {
public:
    static GLFW3 & getInstance() {
        static GLFW3 instance;
        return instance;
    }

    bool init();
    bool poll() const;
    bool terminate();

private:
    GLFW3();
    ~GLFW3();

    struct Data;
    std::unique_ptr<Data> _data;
};

}
