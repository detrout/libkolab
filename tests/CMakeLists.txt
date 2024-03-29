
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DTEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

macro(addTest TEST_NAME)
    QT4_AUTOMOC(${TEST_NAME}.cpp)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} ${QT_QTTEST_LIBRARY} kolab_static)
    add_test(${TEST_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
endmacro()

QT4_AUTOMOC(benchmark.cpp)
add_executable(benchmarktest benchmark.cpp)
target_link_libraries(benchmarktest ${QT_QTTEST_LIBRARY} kolab_static)

addTest(formattest)
addTest(upgradetest)
addTest(kcalconversiontest)
addTest(calendaringtest)
addTest(icalendartest)
addTest(freebusytest)
addTest(kolabobjecttest)
addTest(timezonetest)
addTest(xmlobjecttest)

if(PHP_BINDINGS)
    find_path(PHP_KOLABFORMAT_PATH NAMES kolabformat.php PATHS /usr/local/lib/php/modules /usr/lib/php/modules PATH_SUFFIXES lib/php/modules lib64/php/modules)
    message("php kolabformat include path for testing ${PHP_KOLABFORMAT_PATH}")
    add_test(phptest php -d enable_dl=On -d include_path='.:/usr/share/pear:${CMAKE_BINARY_DIR}:${CMAKE_BINARY_DIR}/calendaring/php:${PHP_KOLABFORMAT_PATH}' -d extension=${CMAKE_BINARY_DIR}/lib/kolabcalendaring.so -d extension=${CMAKE_BINARY_DIR}/lib/kolabshared.so -d extension=${PHP_KOLABFORMAT_PATH}/kolabformat.so ${CMAKE_SOURCE_DIR}/calendaring/php/test.php --verbose)
endif()

