cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(TARGET StarTopologyEmulator)
	return()
endif()

project(StarTopologyEmulator LANGUAGES CXX)

file(GLOB_RECURSE SRC_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Src/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE STAR_TOPOLOGY_EMULATOR_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PUBLIC STAR_TOPOLOGY_EMULATOR_LIBRARY_STATIC)

target_include_directories(${PROJECT_NAME}
	PUBLIC Include/
	PRIVATE Src/)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

option(STE_ENABLE_AFF3CT "Build the AFF3CT physical-layer backend (Aff3ctPhysicalLink)" OFF)

if(STE_ENABLE_AFF3CT)
	set(STE_AFF3CT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/aff3ct" CACHE PATH "Path to the AFF3CT source tree")

	if(NOT EXISTS "${STE_AFF3CT_DIR}/CMakeLists.txt")
		message(FATAL_ERROR
			"STE_ENABLE_AFF3CT=ON but AFF3CT was not found at ${STE_AFF3CT_DIR}.\n"
			"Initialize the submodule: git submodule update --init --recursive")
	endif()

	set(AFF3CT_COMPILE_EXE        OFF CACHE BOOL "" FORCE)
	set(AFF3CT_COMPILE_STATIC_LIB ON  CACHE BOOL "" FORCE)
	set(AFF3CT_COMPILE_SHARED_LIB OFF CACHE BOOL "" FORCE)
	set(SPU_TESTS                 OFF CACHE BOOL "" FORCE)
	set(SPU_STACKTRACE            OFF CACHE BOOL "" FORCE)
	set(SPU_STACKTRACE_SEGFAULT   OFF CACHE BOOL "" FORCE)

	if(MSVC)
		add_compile_options(/bigobj)
	endif()

	add_subdirectory(${STE_AFF3CT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/External/aff3ct EXCLUDE_FROM_ALL)

	target_compile_definitions(${PROJECT_NAME} PUBLIC STE_WITH_AFF3CT)
	target_link_libraries(${PROJECT_NAME} PUBLIC aff3ct-static-lib)
endif()
