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


#include <Exception.h>

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
        if (token.type != Tokenizer::TokenType::ASSIGN && token.type != Tokenizer::TokenType::ASSIGN_LIST) {
            throw Exception("assignment expected");
        }
        variables[name] = Variable(name, token.type == Tokenizer::TokenType::ASSIGN_LIST, tokenizer);
    }
}

Bindings::Bindings(const std::vector<Variable>& variable_list) {
    for (auto& variable: variable_list) {
        variables[variable.name] = variable;
    }
}

void Bindings::print(std::ostream& stream, const std::string& indent) const {
    for (auto& pair : *this) {
        if (!pair.second.is_list) {
            stream << indent;
            pair.second.print_definition(stream);
            stream << std::endl;
        }
    }
}

void Bindings::process(const File& file) {
    for (auto& pair : variables) {
        pair.second.process(file);
    }
}
