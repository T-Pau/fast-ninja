/*
Variable.cc --

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

#include "Variable.h"

Variable::Variable(std::string name, bool is_list, Tokenizer& tokenizer) : name{ std::move(name) }, is_list{ is_list } {
    auto terminator = Tokenizer::TokenType::NEWLINE;
    tokenizer.skip_space();
    if (is_list) {
        auto token = tokenizer.next();
        if (token.type == Tokenizer::TokenType::NEWLINE) {
            token = tokenizer.next();
            if (token.type == Tokenizer::TokenType::BEGIN_SCOPE) {
                terminator = Tokenizer::TokenType::END_SCOPE;
            }
            else {
                tokenizer.unget(token);
                return;
            }
        }
    }

    value = Text{tokenizer, terminator};
}

void Variable::process(const File& file) {
    value.process(file);
}

void Variable::print_definition(std::ostream& stream) const {
    stream << name << " = " << value;
}

void Variable::print_use(std::ostream& stream) const {
    if (is_list) {
        stream << value;
    }
    else {
        stream << "${" << name << "}";
    }
}
