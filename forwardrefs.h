#ifndef FORWARDREFS_H
#define FORWARDREFS_H

#if defined __cplusplus

#ifdef _WIN32
#	ifndef _WIN32_WINNT
#		define  _WIN32_WINNT   0x0501
#	endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#	define IS_WINDOWS	1
#endif

#if defined(__arm__) && defined(__unix__)
#	define IS_ARM_UNIX
#endif

#include <stdint.h>

#define UNUSED(x) (void)x;

#define _release(ptr) if(ptr){delete ptr; ptr=nullptr;}

#define wsep _T(' ')
#ifdef _WIN32
#	include <intsafe.h>
#	include <winsock2.h>
#	include <string>
#	include <vector>
#	include <Windows.h>
#	ifdef _MSC_VER
#		pragma warning(disable: 4996 4995)
#	endif

#	define FT_TICKS_PER_SECOND		((int64)10000000)

#else // #elif defined(__arm__) || defined(__unix__)

#	include <stdio.h>
#	include <unistd.h>
#	include <fcntl.h>

#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/time.h>

#	define FT_TICKS_PER_SECOND		((int64)1000000)
#	define aligned_call16(x)		(x)
#endif
//////////////////////////////////////////////
#include <assert.h>
#include <string>
#include <memory>

// Character interface macros
#ifdef UNICODE
typedef std::wstring string;
#define tx(t) L ## t
#else
typedef std::string string;
#define tx(t) t
#endif

inline float dbl_rand(float l, float r) {
	float l_ = l * 100;
	float r_ = r * 100;
	return static_cast<float>(rand() % int(r_ - l_) + (int)l_) / 100;
}

#endif //__cplusplus

#endif // FORWARDREFS_H
