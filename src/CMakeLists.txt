set(PROGRAM fast-ninja)

ADD_EXECUTABLE(fast-ninja
        fast-ninja.cc
        Bindings.cc
        Build.cc
        File.cc
        Rule.cc
        ScopedDirective.cc
        Text.cc
        Tokenizer.cc
        Variable.cc
)
target_include_directories(fast-ninja PRIVATE ${CMAKE_SOURCE_DIR}/foundation/lib ${PROJECT_BINARY_DIR})
target_link_libraries(fast-ninja foundation)
INSTALL(TARGETS fast-ninja RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
