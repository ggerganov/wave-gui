/*! \file cg_config.h
 *  \brief Some common type definitions.
 *  \author Georgi Gerganov
 */

#pragma once

#ifdef WIN32
    #include <windows.h>

    #ifndef M_PI
        #define M_PI 3.14159265359
    #endif

    #define round(d) (floor((d) + 0.5))

    #define CGSleep Sleep
#else
    #include <unistd.h>

    #define CGSleep usleep
#endif

#define M_iPI  0.31830988618
#define M_iPI2 0.63661977236
#define M_PI2  1.57079632679
#define M_PI4  0.78539816339
#define M_2PI  6.28318530718

//
// DLL stuff
//
#ifdef WIN32

    #ifdef _MSC_VER
        #if _MSC_VER < 1201
            #define for if (0); else for
        #endif
    #endif

    #ifdef MAKE_DLL
        #define CG_EXPORT __declspec(dllexport)
    #else
        #define CG_EXPORT __declspec(dllimport)
    #endif

#else

    #define CG_EXPORT

#endif

/*! \brief Standard floating point number.
 *
 * Use -DDOUBLE switch during compilation to specify that this
 * type is 64-bit floating point. By default we assume 32-bit
 * floating point numbers.
 */
#ifdef SINGLE
    typedef float  CG_Float;
#elif defined DOUBLE
    typedef double CG_Float;
#else
    typedef double CG_Float;
#endif

typedef char CG_I8;
typedef unsigned char CG_UI8;

typedef short CG_I16;
typedef unsigned short CG_UI16;

typedef int   CG_I32;
typedef unsigned int   CG_UI32;
#ifdef WIN32
    typedef __int64 CG_I64;
#else
    typedef long long CG_I64;
#endif

// Units
#define _MM_ (1e-3)
#define _M_ (1e0)
#define _KM_ (1e3)

#include <cstddef>
#include <exception>

namespace CG {
/*! \brief An Exception class.
 *
 * Used for propagating unexpected errors. An information
 * string with each Exception can also be provided.
 */
class CG_EXPORT Exception : public std::exception {
public:
    /*! \brief The Constructor. */
    Exception(const char *info, ...);

    /*! \brief The Destructor. */
    ~Exception() throw();

    char _info[1024]; //!< Information about the Exception
};
}
