/*
Bindings.cc --

Copyright (C) Dieter Baron

The authors can be contacted at <assembler@tpau.group>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The names of the authors may not be used to endorse or promote
  products derived from this software without specific prior
  written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Bindings.h"

#include <algorithm>

#include "Exception.h"
#include "FilenameVariable.h"
#include "TextVariable.h"
#include "VariableDependencies.h"

Bindings::Bindings(Tokenizer& tokenizer) {
    auto token = tokenizer.next(Tokenizer::Skip::WHITESPACE);
    if (token.type != Tokenizer::TokenType::BEGIN_SCOPE) {
        tokenizer.unget(token);
        return;
    }

    while (((token = tokenizer.next(Tokenizer::Skip::SPACE))) && token.type != Tokenizer::TokenType::END_SCOPE) {
        if (token.type != Tokenizer::TokenType::WORD) {
            throw Exception("invalid variable name");
        }
        auto name = token.string();
        token = tokenizer.next(Tokenizer::Skip::SPACE);
        if (token.type == Tokenizer::TokenType::ASSIGN) {
            variables[name] = std::unique_ptr<Variable>(new TextVariable{name, tokenizer});
        }
        else if (token.type == Tokenizer::TokenType::ASSIGN_LIST) {
            variables[name] = std::unique_ptr<Variable>(new FilenameVariable{name, tokenizer});
        }
        else {
            throw Exception("assignment expected");
        }
    }
}

void Bindings::print(std::ostream& stream, const std::string& indent) const {
    auto variable_names = std::vector<std::string>{};

    for (auto& pair : *this) {
        variable_names.emplace_back(pair.first);
    }

    std::sort(variable_names.begin(), variable_names.end());

    for (const auto& variable: variable_names) {
        stream << indent;
        variables.find(variable)->second->print_definition(stream);
    }
}

void Bindings::resolve(const Scope& scope, bool expand_variables, bool classify_filenames) {
    auto dependencies = VariableDependencies(variables);
    ResolveResult result;
    auto context = ResolveContext{scope, result, expand_variables, classify_filenames};

    while (!dependencies.finished()) {
        for (auto& name: dependencies.get_next()) {
            result.unresolved_used_variables.clear();
            variables[name]->resolve(context);
            dependencies.update(name, result.unresolved_used_variables);
        }
    }
}
