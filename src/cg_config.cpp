/*! \file cg_config.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_config.h"

#include <cstdio>
#include <cstdarg>

CG::Exception::Exception(const char *info, ...) {
    //_info = new char [1024];
    va_list ap;
    va_start(ap, info);
    vsprintf(_info, info, ap);
    va_end(ap);
}

CG::Exception::~Exception() throw() {}
