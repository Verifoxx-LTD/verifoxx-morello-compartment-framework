# Copyright (C) 2024 Verifoxx Limited.  All rights reserved.
# CMakeLists to build the capability manager and compartment library for the example
cmake_minimum_required (VERSION 3.19)

project("compartment-framework" LANGUAGES C CXX ASM)

include(CheckPIESupported)
include(CMakeForceCompiler)

check_pie_supported()

set (CAPMGR "cap-mgr")
set (COMPLIB "compartment")

add_executable (${CAPMGR})

set (CMAKE_SYSTEM_NAME Linux)
set (CMAKE_SYSTEM_VERSION 1)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Are we doing a static build of the cap mgr exe or dynamic?
if (NOT DEFINED CAPMGR_BUILD_STATIC)
    set (CAPMGR_BUILD_STATIC 0)
endif()

# Search path for libraries - which is also where targets are placed
if (NOT DEFINED MORELLO_PURECAP_LIBS_FOLDER)
	set(MORELLO_PURECAP_LIBS_FOLDER "/purecap-lib")
endif ()

message (STATUS "RPATH set to ${MORELLO_PURECAP_LIBS_FOLDER}")

# Toolchain check - only GNU + GlibC supported currently
if ( NOT DEFINED CHERI_GNU_TOOLCHAIN_DIR OR NOT CHERI_GNU_TOOLCHAIN_DIR )
	message(FATAL_ERROR "Valid toolchain has not been selected - cannot continue")
endif ()

# Only Gnu toolchain supported currently
message(STATUS "*** GNU TOOLCHAIN SELECTED ***")

# Search paths
set (CMAKE_FIND_ROOT_PATH "${CHERI_GNU_TOOLCHAIN_DIR}")
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

message(STATUS "Toolchain binaries=${CHERI_GNU_TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}-*")
message(STATUS "CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")

if (NOT DEFINED CMAKE_SYS_ROOT)
    set(CMAKE_SYS_ROOT "${CHERI_GNU_TOOLCHAIN_DIR}/libc")
    message(STATUS "CMAKE_SYS_ROOT=${CHERI_GNU_TOOLCHAIN_DIR}")
endif ()

if (NOT DEFINED CMAKE_C_COMPILER)
    set (CMAKE_C_COMPILER "${CHERI_GNU_TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}-gcc")
endif ()

if (NOT_DEFINED_CMAKE_CXX_COMPLIER)
    set (CMAKE_CXX_COMPILER "${CHERI_GNU_TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}-g++")
endif ()

# Set Build flags for purecap
set (CHERI_ARCH_ABI "-march=morello+c64 -mabi=purecap")
set (LINK_OPTIONS_SETTINGS "-Wl,-rpath,${MORELLO_PURECAP_LIBS_FOLDER}")

# _GNU_SOURCE is required since we build some glibc internals
target_compile_definitions(${CAPMGR} PRIVATE _GNU_SOURCE=1)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CHERI_ARCH_ABI}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CHERI_ARCH_ABI}")
set (CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

# Reset default linker flags
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

set (CMAKE_C_STANDARD 99)
set (CMAKE_CXX_STANDARD 14)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif ()
message(STATUS "Build Type=${CMAKE_BUILD_TYPE}")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wformat -O0")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat -O0")

if ( CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release build so stripping debug symbols")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--strip-debug")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--strip-debug")
endif ()

# Add compile flag CAPMGR_BUILD_STATIC since this affects the way we do symbol resolution patches
if (CAPMGR_BUILD_STATIC)
    target_compile_definitions(${CAPMGR} PRIVATE CAPMGR_BUILT_STATIC_ENABLE=1)
else ()
    target_compile_definitions(${CAPMGR} PRIVATE CAPMGR_BUILT_STATIC_ENABLE=0)
endif ()

##### Find all of our source code, using macro from macros.cmake
include(${CMAKE_CURRENT_LIST_DIR}/macros.cmake)

# Capmgr folder
set (CAPMGR_AND_COMPARTMENTS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Capmgr files
ALL_FILES_AND_INC_FOLDERS(${CAPMGR_AND_COMPARTMENTS_DIR}/capmgr CAPMGR_FILES CAPMGR_INC_FOLDERS)

# Utils files
ALL_FILES_AND_INC_FOLDERS(${CAPMGR_AND_COMPARTMENTS_DIR}/utils UTILS_FILES UTILS_INC_FOLDERS)

# Common files
ALL_FILES_AND_INC_FOLDERS(${CAPMGR_AND_COMPARTMENTS_DIR}/common COMMON_FILES COMMON_INC_FOLDERS)

# Compartment only files
ALL_FILES_AND_INC_FOLDERS(${CAPMGR_AND_COMPARTMENTS_DIR}/compartment COMPARTMENT_FILES COMPARTMENT_INC_FOLDERS)

# Our example for demo
set (EXAMPLES_FOLDER ${CAPMGR_AND_COMPARTMENTS_DIR}/example_usage)

###### Compartment Library Files ########
add_library (${COMPLIB} SHARED
    ${COMMON_FILES}
    ${COMPARTMENT_FILES}
    ${EXAMPLES_FOLDER}/example_comp_api_impl.cpp
    ${EXAMPLES_FOLDER}/example_comp_api.h
    ${EXAMPLES_FOLDER}/example_capmgr_service_api.h
)
target_include_directories(${COMPLIB} PRIVATE
    ${COMMON_INC_FOLDERS}
    ${COMPARTMENT_INC_FOLDERS}
    ${EXAMPLES_FOLDER}
)

# Compartment lib is built with RPATH so dependencies can be found
target_link_options(${COMPLIB} BEFORE PRIVATE -static-libstdc++ -static-libgcc -Wl,-rpath,${MORELLO_PURECAP_LIBS_FOLDER})
target_link_libraries (${COMPLIB} -Wl,-Bstatic,-lpthread -Wl,-Bstatic,-ldl -Wl,-Bdynamic,-lm -Wl,-Bdynamic,-lc)


###### Capability Manager Executable ##########
add_dependencies(${CAPMGR} ${COMPLIB})

set_target_properties (${CAPMGR} PROPERTIES POSITION_INDEPENDENT_CODE ON LINKER_LANGUAGE CXX)

# Let's keep the map file
target_link_options(${CAPMGR} PUBLIC "SHELL:-Xlinker -Map" "SHELL:-Xlinker ${CAPMGR}.map")

## CapMgr Files ##
target_sources(${CAPMGR} PRIVATE
    ${CAPMGR_FILES}
    ${UTILS_FILES}
    ${COMMON_FILES}
    ${PLATFORM_SHARED_CAPMGR_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${EXAMPLES_FOLDER}/example_capmgr_service_api_impl.cpp
    ${EXAMPLES_FOLDER}/example_capmgr_service_api.h
    ${EXAMPLES_FOLDER}/example_comp_api.h
)

target_include_directories(${CAPMGR} PRIVATE
    ${CAPMGR_INC_FOLDERS}
    ${UTILS_INC_FOLDERS}
    ${COMMON_INC_FOLDERS}
    ${PLATFORM_SHARED_INCLUDE_CAPMGR_FOLDER}
    ${EXAMPLES_FOLDER}
)

if (CAPMGR_BUILD_STATIC)
    target_link_options(${CAPMGR} BEFORE PRIVATE ${LINK_OPTIONS_SETTINGS} -static-libstdc++ -static-libgcc)
    target_link_libraries(${CAPMGR} -lpthread -ldl -lm -lc -static)
else ()
    target_link_options(${CAPMGR} BEFORE PRIVATE ${LINK_OPTIONS_SETTINGS} -static-libstdc++ -static-libgcc -Wl,-rpath,${MORELLO_PURECAP_LIBS_FOLDER})
    target_link_libraries(${CAPMGR} -Wl,-Bstatic,-lpthread -Wl,-Bstatic,-ldl -Wl,-Bdynamic,-lm -Wl,-Bdynamic,-lc)
endif ()

##### Install ###
install (TARGETS ${CAPMGR} DESTINATION bin)
install (TARGETS ${COMPLIB} DESTINATION lib)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${CAPMGR}.map DESTINATION bin)
