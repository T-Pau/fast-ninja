/*
Text.cc --

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

#include "Text.h"


#include "File.h"
#include <Exception.h>

Text::Text(Tokenizer& tokenizer, Tokenizer::TokenType terminator) {
    tokenizer.skip_space();
    while (const auto token = tokenizer.next()) {
        switch (token.type) {
            case Tokenizer::TokenType::NEWLINE:
                if (terminator == Tokenizer::TokenType::COLON) {
                    throw Exception("missing ':'");
                }
                else if (terminator == Tokenizer::TokenType::END_SCOPE) {
                    emplace_back(" ", false);
                }
                else {
                    return;
                }
                break;

            case Tokenizer::TokenType::END_SCOPE:
                return;

            case Tokenizer::TokenType::VARIABLE_REFERENCE:
                emplace_back(token.value, true);
                break;

            case Tokenizer::TokenType::COLON:
                if (terminator == Tokenizer::TokenType::COLON) {
                    return;
                }
                // fallthrough
            default:
                emplace_back(token.string(), false);
                break;
        }
    }
}

std::ostream& operator<<(std::ostream& stream, const Text& text) {
    text.print(stream);
    return stream;
}

void Text::print(std::ostream& stream) const {
    for (const auto& element : *this) {
        if (element.is_variable) {
            if (element.variable) {
                element.variable->print_use(stream);
            }
            else {
                stream << "$" << element.value;
            }
        }
        else {
            stream << element.value;
        }
    }
}

void Text::process(const File& file) {
    for (auto& element : *this) {
        if (element.is_variable) {
            element.variable = file.find_variable(element.value);
        }
    }
}

void Text::collect_words(std::unordered_set<std::string>& words) const {
    for (auto& element : *this) {
        if (element.is_variable) {
            if (element.variable && element.variable->is_list) {
                element.variable->value.collect_words(words);
            }
        }
        else {
            words.insert(element.value);
        }
    }
}

std::string Text::string() const {
    // TODO: implement
    return {};
}
