/*
TextParser.cc --

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

#include "TextParser.h"


#include "File.h"
#include <Exception.h>

Text TextParser::parse() {
    tokenizer.skip_space();
    parse_tokens();
    end_word();
    return std::move(text);
}

void TextParser::parse_tokens() {
    while (const auto token = tokenizer.next()) {
        switch (token.type) {
            case Tokenizer::TokenType::NEWLINE:
                if (terminator == Tokenizer::TokenType::COLON) {
                    throw Exception("missing ':'");
                }
                else if (terminator == Tokenizer::TokenType::END_SCOPE) {
                    end_word();
                    text.emplace_back(Text::ElementType::WHITESPACE, " ");
                }
                else {
                    return;
                }
                break;

            case Tokenizer::TokenType::END_SCOPE:
                return;

            case Tokenizer::TokenType::VARIABLE_REFERENCE:
                if (token.value == "in" || token.value == "out") {
                    end_word(true);
                    text.emplace_back(Text::ElementType::VARIABLE, token.value);
                }
                else if (const auto variable = file.find_variable(token.value)) {
                    if (variable->is_list) {
                        if (!current_word.empty()) {
                            // TODO: warn: expanding list variable in middle of word
                            end_word();
                        }
                        text.append(variable->value);
                    }
                    else {
                        current_word += variable->value.string();
                    }
                }
                break;

            case Tokenizer::TokenType::COLON:
            case Tokenizer::TokenType::IMPLICIT_DEPENDENCY:
            case Tokenizer::TokenType::ORDER_DEPENDENCY:
            case Tokenizer::TokenType::VALIDATION_DEPENDENCY:
                if (terminator == token.type) {
                    return;
                }
                text.emplace_back(Text::ElementType::PUNCTUATION, token.value);
                break;

            case Tokenizer::TokenType::SPACE:
                end_word();
                text.emplace_back(Text::ElementType::WHITESPACE, token.value);
                break;

            default:
                text.emplace_back(Text::ElementType::WORD, token.value);
                break;
        }
    }
}

void TextParser::end_word(bool partial) {
    if (!current_word.empty()) {
        text.emplace_back(partial || current_word_is_partial ? Text::ElementType::PARTIAL : Text::ElementType::WORD, current_word);
    }
    current_word_is_partial = partial;
}
