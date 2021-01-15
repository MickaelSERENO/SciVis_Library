#Minimum cmake version 3.0
cmake_minimum_required(VERSION 3.0)

#Define the project
project(SerenoVTKParser CXX)

#Define Outputs (bin, lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set(CMAKE_CXX_STANDARD 17)

set(INSTALL_LIB_DIR       "${CMAKE_INSTALL_PREFIX}/lib"           CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR       "${CMAKE_INSTALL_PREFIX}/include"       CACHE PATH "Installation directory for headers")
set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")
set(RELEASE               FALSE                                   CACHE BOOL "Compiling in release mode.")
set(COMPILE_OPENCL        FALSE                                   CACHE BOOL "Compile the OpenCL module?")

set(COMPILE_TEST         FALSE CACHE BOOL "Should we compile the test program ?")
if(MSVC)
    set(COMPILE_C_SHARP_TEST FALSE CACHE BOOL "Should we compile the C# binding ?")
endif()

set(VERSION 1.0)

#Debug/Release version version
if(RELEASE)
	MESSAGE(STATUS "Compiling in release mode")
	set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
else()
	MESSAGE(STATUS "Compiling in Debug mode")
	set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif()
set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)
set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /w14244")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

#Workaround for MSVC not creating DEBUG and RELEASE folder
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

#Set the sources and the headers
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
set(PC ${CMAKE_BINARY_DIR}/serenoSciVis.pc)

list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)


#Handles PKG Config files
find_package(PkgConfig REQUIRED)
PKG_CHECK_MODULES(VTKPARSER REQUIRED serenoVTKParser)
PKG_CHECK_MODULES(MATH      REQUIRED serenoMath)
PKG_CHECK_MODULES(GLM       REQUIRED glm)

#Check OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#Configure the executable : sources, compile options (CFLAGS) and link options (LDFLAGS)
add_library(serenoSciVis SHARED ${SOURCES} ${HEADERS})

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_definitions(-DSNAPSHOT)
endif()

#Check the OpenCL module.
if(COMPILE_OPENCL)
    add_definitions(-DCOMPILE_OPENCL)
    PKG_CHECK_MODULES(OPENCL REQUIRED OpenCL)
    target_compile_options(serenoSciVis PUBLIC ${OPENCL_CFLAGS})
    target_link_libraries(serenoSciVis PUBLIC ${OPENCL_LDFLAGS})
    MESSAGE(STATUS "Compiling with OPENCL support")
endif()

target_compile_options(serenoSciVis PUBLIC ${VTKPARSER_CFLAGS} ${GLM_CFLAGS}  ${MATH_CFLAGS})
target_link_libraries(serenoSciVis PUBLIC ${VTKPARSER_LDFLAGS} ${GLM_LDFLAGS} ${MATH_LDFLAGS} -lm)

#Add include directory
target_include_directories(serenoSciVis PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

#Configure .pc
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/serenoSciVis.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/serenoSciVis.pc @ONLY)

#Test
if(COMPILE_TEST)
    add_executable(serenoSciVisTest ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(serenoSciVisTest PUBLIC serenoSciVis)
endif()

#Installation
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS serenoSciVis
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    file(GLOB DEPLOY_FILES_AND_DIRS "${PROJECT_SOURCE_DIR}/include/*")
    foreach(ITEM ${DEPLOY_FILES_AND_DIRS})
       IF( IS_DIRECTORY "${ITEM}" )
          LIST( APPEND DIRS_TO_DEPLOY "${ITEM}" )
       ELSE()
          LIST( APPEND FILES_TO_DEPLOY "${ITEM}" )
       ENDIF()
    endforeach()
    install(FILES ${FILES_TO_DEPLOY} DESTINATION ${INSTALL_INC_DIR})
    install(DIRECTORY ${DIRS_TO_DEPLOY} DESTINATION ${INSTALL_INC_DIR})
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    install(FILES ${PC} DESTINATION "${INSTALL_PKGCONFIG_DIR}")
endif()

#if(COMPILE_C_SHARP_TEST AND MSVC)
#	enable_language(CSharp)
#	include(CSharpUtilities)
#	add_executable(serenoSciVisCSharpTest ${CMAKE_CURRENT_SOURCE_DIR}/Bindings/C\#/VTKParser.cs)
#	target_compile_options(serenoSciVisCSharpTest PUBLIC "/unsafe")
#	target_link_libraries(serenoSciVisCSharpTest PUBLIC serenoSciVis)
#endif()
