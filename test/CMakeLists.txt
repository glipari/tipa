include_directories (.)
include_directories (../src)

set (TIPA_TEST_LIBRARY ${PROJECT_NAME}_test)

# Create Catch library to decrease compile times
add_library (${TIPA_TEST_LIBRARY} ${LIBRARY_TYPE} test_main.cpp)

# Define a macro to simplify tests creation
function (create_test name)
    add_executable (${name} ${ARGN})
    target_compile_features (${name} PRIVATE cxx_range_for)
    target_link_libraries (${name} ${TIPA_TEST_LIBRARY} ${PROJECT_NAME})
    add_test (NAME ${name} COMMAND ${name})
endfunction (create_test)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/struct.txt
    ${CMAKE_CURRENT_BINARY_DIR}/struct.txt COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lexer_multi.txt
    ${CMAKE_CURRENT_BINARY_DIR}/lexer_multi.txt COPYONLY)

create_test (TestCollector test_collector.cpp)
create_test (TestLexer     test_lexer.cpp)
create_test (TestLexer2    test_lexer2.cpp)
create_test (TestFile      test_file.cpp)
create_test (TestParser    test_parser.cpp)
create_test (TestWptr      test_wptr.cpp)
create_test (TestVisitor   test_visitor.cpp)
