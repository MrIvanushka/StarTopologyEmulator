#pragma once

#ifdef STAR_TOPOLOGY_EMULATOR_LIBRARY
#	ifndef STAR_TOPOLOGY_EMULATOR_LIBRARY_STATIC
#		if defined _WIN32 || defined __CYGWIN__ || defined __MINGW32__
#			define STAR_TOPOLOGY_EMULATOR_LIB_EXPORT __declspec(dllexport)
#		else
#			define STAR_TOPOLOGY_EMULATOR_LIB_EXPORT __attribute__((visibility("default")))
#		endif
#	else
#		define STAR_TOPOLOGY_EMULATOR_LIB_EXPORT
#	endif
#else
#	ifndef STAR_TOPOLOGY_EMULATOR_LIBRARY_STATIC
#		if defined _WIN32 || defined __CYGWIN__ || defined __MINGW32__
#			define STAR_TOPOLOGY_EMULATOR_LIB_EXPORT __declspec(dllimport)
#		else
#			define STAR_TOPOLOGY_EMULATOR_LIB_EXPORT __attribute__((visibility("default")))
#		endif
#	else
#		define STAR_TOPOLOGY_EMULATOR_LIB_EXPORT
#	endif
#endif
