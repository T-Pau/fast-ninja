
set(XFAIL_TESTS
)

if(RUN_REGRESS)
file(GLOB TESTS ${CMAKE_CURRENT_SOURCE_DIR}/*.test)
foreach(FULL_CASE IN LISTS TESTS)
    get_filename_component(CASE ${FULL_CASE} NAME)
    add_test(NAME ${CASE} COMMAND nihtest ${CASE})
    set_tests_properties(${CASE} PROPERTIES SKIP_RETURN_CODE 77)
endforeach()

foreach(CASE ${XFAIL_TESTS})
    set_tests_properties(${CASE} PROPERTIES WILL_FAIL TRUE)
endforeach()
endif()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/nihtest.conf.in ${CMAKE_CURRENT_BINARY_DIR}/nihtest.conf)
