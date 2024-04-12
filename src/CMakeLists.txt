set(PROGRAM fast-ninja)

ADD_EXECUTABLE(fast-ninja
        fast-ninja.cc
        Bindings.cc
        Build.cc
        Dependencies.cc
        File.cc
        Filename.cc
        FilenameList.cc
        FilenameVariable.cc
        FilenameWord.cc
        Pool.cc
        ResolveContext.cc
        Rule.cc
        Scope.cc
        ScopedDirective.cc
        Text.cc
        TextVariable.cc
        Tokenizer.cc
        Variable.cc
        VariableReference.cc
        Word.cc
)
target_include_directories(fast-ninja PRIVATE ${CMAKE_SOURCE_DIR}/foundation/lib ${PROJECT_BINARY_DIR})
target_link_libraries(fast-ninja foundation)
INSTALL(TARGETS fast-ninja RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
