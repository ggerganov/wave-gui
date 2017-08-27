/*! \file cg_timer.h
 *  \brief A timer class used for measuring time.
 *  \author Georgi Gerganov
 */

#pragma once

#include "cg_config.h"

namespace CG {

#ifndef DOXYGEN_HIDE
    class TimerImplementation;
#endif

/*! \brief A timer class used for measuring time. */
class CG_EXPORT Timer {
public:
    /*! \brief The Constructor. */
    Timer();

    /*! \brief The Destructor. */
    ~Timer();

    /*! \brief Start measuring time. */
    void start();

    /*! \brief Get elapsed time.
     *
     * @return The elapsed time in seconds.
     */
    float time();

private:
#ifndef DOXYGEN_HIDE
    TimerImplementation *m_timer; //!< Private implementation for multi-platform support
    Timer(const Timer&) {};       //!< Private Constructor
#endif
};
}
