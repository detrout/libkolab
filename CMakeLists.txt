project(Libkolab)

cmake_minimum_required(VERSION 2.6.4)

option( BUILD_TESTS "Build the tests" TRUE )
option( PYTHON_BINDINGS "Build bindings for python" FALSE )
option( PHP_BINDINGS "Build bindings for php" FALSE )
option( USE_LIBCALENDARING "Use libcalendaring" FALSE )

set(Libkolab_MODULE_DIR ${Libkolab_SOURCE_DIR}/cmake/modules)
set(CMAKE_MODULE_PATH ${Libkolab_MODULE_DIR})

# only available from cmake-2.8.0
if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 7)
    cmake_policy(SET CMP0012 NEW)
endif()

# only available from cmake-2.8.4
if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 7 AND
    ${CMAKE_PATCH_VERSION} GREATER 3)
    cmake_policy(SET CMP0017 NEW)
endif()

# Versioning
# x.y.z scheme
# Development versions are only x.y
#
# i.e.
# 0.1 (0.1 development version towards 0.1.0)
# 0.1.0 (first release)
# 0.1.1 (patch release for 0.1.0)
# 0.2 (0.2 development version towards 0.2.0)
set(Libkolab_VERSION_MAJOR 0)
set(Libkolab_VERSION_MINOR 4)
# Enable the full x.y.z version only for release versions
set(Libkolab_VERSION_PATCH 2)
set(Libkolab_VERSION ${Libkolab_VERSION_MAJOR}.${Libkolab_VERSION_MINOR}.${Libkolab_VERSION_PATCH} )
#set(Libkolab_VERSION ${Libkolab_VERSION_MAJOR}.${Libkolab_VERSION_MINOR} )
set(Libkolab_VERSION_STRING ${CMAKE_PROJECT_NAME}-${Libkolab_VERSION})

# set up install directories.
set(LIB_INSTALL_DIR lib CACHE STRING "The directories where to install libraries to")
set(INCLUDE_INSTALL_DIR include CACHE STRING "The directory where to install headers to")
set(INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR}/kolab)
set(CMAKECONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/Libkolab )

include(MacroLogFeature)

# Do the building
find_package(Libkolabxml 0.7 REQUIRED)
macro_log_feature(Libkolabxml_FOUND "Libkolabxml" "Kolab XML Format 3 serializing library" "http://git.kolab.org/libkolabxml/" TRUE "0.7" "Required for reading/writing Kolab XML Objects")

find_package(Qt4 4.6.0 REQUIRED)

if (USE_LIBCALENDARING)
    find_package(Libcalendaring)
    macro_log_feature(LIBCALENDARING_FOUND "Libcalendaring" "KDE Calendaring-Dependencies Library" "http://git.kolab.org/libcalendaring/" TRUE "" "Required if KDE libraries are not available.")
else()
    find_package(KDE4 4.8 REQUIRED)
    find_package(KdepimLibs 4.8 REQUIRED)
endif()

find_package(SWIG)

#Show summary of found libraries
macro_display_feature_log()

# add_definitions(-DKDEPIMLIBS_VERSION=0x${KdepimLibs_VERSION_MAJOR}0${KdepimLibs_VERSION_MINOR}${KDEPIMLIBS_VERSION_PATCH})
# add_definitions( -DKDEPIMLIBS_VERSION=((${KdepimLibs_VERSION_MAJOR}<<16)|(${KdepimLibs_VERSION_MINOR}<<8)|(${KDEPIMLIBS_VERSION_PATCH})) )

if("${KdepimLibs_VERSION}" VERSION_GREATER "4.8.40" OR USE_LIBCALENDARING)
    add_definitions(-DKDEPIMLIBS_VERSION_DEVEL)
endif()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wformat-security -fno-exceptions -DQT_NO_EXCEPTIONS -fno-check-new -fno-common -Woverloaded-virtual -fno-threadsafe-statics -fvisibility=hidden -Werror=return-type -fvisibility-inlines-hidden -fexceptions -UQT_NO_EXCEPTIONS -fPIC -g" )
# message("${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG")

if (USE_LIBCALENDARING)
    set( KDE_INCLUDES ${Libcalendaring_INCLUDE_DIRS} )
    set( KDE_LIBRARIES ${Libcalendaring_LIBRARIES} )
    message("${Libcalendaring_INCLUDE_DIRS} ${Libcalendaring_LIBRARIES}")
else()
    set( KDE_INCLUDES ${KDEPIMLIBS_INCLUDE_DIRS} ${KDE4_INCLUDE_DIR})
    set( KDE_LIBRARIES
        ${KDEPIMLIBS_KCALCORE_LIBS}
        ${KDEPIMLIBS_KABC_LIBS}
        ${KDEPIMLIBS_KMIME_LIBS}
        ${KDEPIMLIBS_AKONADI_LIBS}
        ${KDEPIMLIBS_AKONADI_NOTES_LIBS}
        ${KDEPIMLIBS_KCALUTILS_LIBS}
        ${KDEPIMLIBS_KPIMUTILS_LIBS}
        ${KDE4_KDECORE_LIBRARY}
        ${KDE4_KIO_LIBRARY}
        )
endif()

find_package(Boost REQUIRED)

include_directories(    
    ${QT_INCLUDES}
    ${KDE_INCLUDES}
    ${CMAKE_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/kolabformatV2
    ${Libkolabxml_INCLUDES}
    ./
)

configure_file(libkolab-version.h.cmake "${CMAKE_BINARY_DIR}/libkolab-version.h" @ONLY)

add_subdirectory(kolabformatV2)
add_subdirectory(conversion)
add_subdirectory(calendaring)
add_subdirectory(icalendar)
add_subdirectory(freebusy)

QT4_WRAP_CPP(CALENDARING_MOC calendaring/event.h)
# QT4_WRAP_CPP(CONVERSION_MOC conversion/qtevent.h conversion/qtduration.h)

set(KOLAB_SRCS
    kolabformat/kolabobject.cpp
    kolabformat/xmlobject.cpp
    kolabformat/formathelpers.cpp
    kolabformat/errorhandler.cpp
    kolabformat/v2helpers.cpp
    mime/mimeutils.cpp
    ${CONVERSION_SRCS}
    ${kolabformatv2_SRCS}
    ${CALENDARING_SRCS}
    ${ICALENDAR_SRCS}
    ${CALENDARING_MOC}
    ${CONVERSION_MOC}
    ${FREEBUSY_SRCS}
)

set(KOLAB_LINK_LIBRARIES
    ${Libkolabxml_LIBRARIES}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE_LIBRARIES}
)
if(BUILD_TESTS)
    #for tests only
    enable_testing()
    add_library(kolab_static STATIC ${KOLAB_SRCS})
    target_link_libraries(kolab_static ${KOLAB_LINK_LIBRARIES})
    add_subdirectory(tests)
endif(BUILD_TESTS)

add_library(kolab SHARED ${KOLAB_SRCS})
target_link_libraries(kolab ${KOLAB_LINK_LIBRARIES})
set_target_properties(kolab PROPERTIES VERSION ${Libkolab_VERSION}
                                     SOVERSION ${Libkolab_VERSION_MAJOR} )

install(TARGETS kolab EXPORT LibkolabExport 
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

install(FILES
    kolab_export.h
    kolabformat/kolabdefinitions.h
    kolabformat/formathelpers.h
    kolabformat/kolabobject.h
    kolabformat/errorhandler.h
    conversion/kcalconversion.h
    conversion/kabcconversion.h
    conversion/commonconversion.h
    freebusy/freebusy.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
)

install(EXPORT LibkolabExport DESTINATION ${CMAKECONFIG_INSTALL_DIR} FILE LibkolabTargets.cmake )

file(RELATIVE_PATH relInstallDir ${CMAKE_INSTALL_PREFIX}/${CMAKECONFIG_INSTALL_DIR} ${CMAKE_INSTALL_PREFIX} )

configure_file(${Libkolab_MODULE_DIR}/LibkolabConfig.cmake.in ${Libkolab_BINARY_DIR}/LibkolabConfig.cmake @ONLY )
configure_file(${Libkolab_MODULE_DIR}/LibkolabConfigVersion.cmake.in ${Libkolab_BINARY_DIR}/LibkolabConfigVersion.cmake @ONLY )
# Install these two files into the same directory as the generated exports-file.
install(FILES ${Libkolab_BINARY_DIR}/LibkolabConfig.cmake ${Libkolab_BINARY_DIR}/LibkolabConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR} )


include(SWIGUtils)
if(PYTHON_BINDINGS)
    generatePythonBindings(shared shared.i)
endif(PYTHON_BINDINGS)

if(PHP_BINDINGS)
    generatePHPBindings(kolabshared shared.i)
    add_subdirectory(kolabformat/php)
endif(PHP_BINDINGS)
