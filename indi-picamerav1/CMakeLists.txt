cmake_minimum_required(VERSION 2.4.7)
PROJECT(indi_picameraV1 CXX C)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

include(GNUInstallDirs)

set (GENERIC_VERSION_MAJOR 0)
set (GENERIC_VERSION_MINOR 1)

find_package(Threads REQUIRED)
find_package(CFITSIO REQUIRED)
find_package(INDI REQUIRED)
find_package(ZLIB REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/indi_picameraV1.xml.cmake ${CMAKE_CURRENT_BINARY_DIR}/indi_picameraV1.xml )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${INDI_INCLUDE_DIR})
include_directories( ${CFITSIO_INCLUDE_DIR})

include(CMakeCommon)

############# INDI PICAMERA ###############
set(indipicamera_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/indi_picameraV1.cpp
)

add_executable(indi_picameraV1 ${indipicamera_SRCS})

target_link_libraries(indi_picameraV1 ${INDI_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CFITSIO_LIBRARIES} m ${ZLIB_LIBRARY})

install(TARGETS indi_picameraV1 RUNTIME DESTINATION bin)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/indi_picameraV1.xml DESTINATION ${INDI_DATA_DIR})
