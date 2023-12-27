/*
Tokenizer.cc --

Copyright (C) Dieter Baron

The authors can be contacted at <fast-ninja@tpau.group>

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

#include "Tokenizer.h"

#include <algorithm>

#include "Exception.h"

// clang-format off
std::unordered_map<std::string, Tokenizer::TokenType> Tokenizer::keywords = {
    {"build", TokenType::BUILD},
    {"default", TokenType::DEFAULT},
    {"include", TokenType::INCLUDE},
    {"pool", TokenType::POOL},
    {"rule", TokenType::RULE},
    {"subninja", TokenType::SUBNINJA}
};

std::unordered_map<int, Tokenizer::CharacterType> Tokenizer::special_characters = {
    {EOF, CharacterType::END},
    {' ', CharacterType::SPACE},
    {'\t', CharacterType::ILLEGAL},
    {'\n', CharacterType::NEWLINE},
    {'$', CharacterType::PUNCTUATION},
    {'-', CharacterType::SIMPLE_VARIABLE},
    {'.', CharacterType::BRACED_VARIABLE},
    {':', CharacterType::PUNCTUATION},
    {'=', CharacterType::PUNCTUATION},
    {'@', CharacterType::PUNCTUATION},
    {'_', CharacterType::SIMPLE_VARIABLE},
    {'|', CharacterType::PUNCTUATION},
};
// clang-format on

Tokenizer::Tokenizer(const std::filesystem::path& filename): filename{filename}, source{filename} {
    if (source.fail()) {
        throw Exception("can't open '%s'", filename.c_str());
    }
}

std::string Tokenizer::Token::string() const {
    if (type == TokenType::VARIABLE_REFERENCE) {
        return "$" + value;
    }
    else if (value.empty()) {
        return type_name();
    }
    else {
        return value;
    }
}

std::string Tokenizer::Token::type_name(TokenType type) {
    switch (type) {
        case TokenType::ASSIGN:
            return "=";

        case TokenType::ASSIGN_LIST:
            return ":=";

        case TokenType::BEGIN_SCOPE:
            return "{";

        case TokenType::BUILD:
            return "build";

        case TokenType::COLON:
            return ":";

        case TokenType::DEFAULT:
            return "default";

        case TokenType::END:
            return "END";

        case TokenType::END_SCOPE:
            return "}";

        case TokenType::IMPLICIT_DEPENDENCY:
            return "|";

        case TokenType::INCLUDE:
            return "include";

        case TokenType::NEWLINE:
            return "newline";

        case TokenType::ORDER_DEPENDENCY:
            return "|@";

        case TokenType::POOL:
            return "pool";

        case TokenType::RULE:
            return "rule";

        case TokenType::SPACE:
            return "space";

        case TokenType::SUBNINJA:
            return "subninja";

        case TokenType::VALIDATION_DEPENDENCY:
            return "||";

        case TokenType::VARIABLE_REFERENCE:
            return "$";

        case TokenType::WORD:
            return "word";
    }
}

Tokenizer::Token Tokenizer::expect(TokenType type, Skip skip) {
    const auto token = next(skip);
    if (token.type != type) {
        throw Exception("expected %s", Token::type_name(type).c_str());
    }
    return token;
}

Tokenizer::Token Tokenizer::next(Skip skip) {
    switch (skip) {
        case Skip::NONE:
            break;

        case Skip::SPACE:
            skip_space();
            break;

        case Skip::WHITESPACE:
            skip_whitespace();
            break;
    }

    return get_next();
}

Tokenizer::Token Tokenizer::get_next() {
    if (ungot) {
        auto token = *ungot;
        ungot.reset();
        return token;
    }

    if (beggining_of_line) {
        line_number += 1;
    }

    while (true) {
        switch (const auto c = source.get()) {
            case EOF:
                return Token{ TokenType::END };

            case '\t':
                throw Exception{ "tabs are not allowed, use spaces" };

            case '\n':
                if (beggining_of_line) {
                    if (indent > 0) {
                        indent = 0;
                        source.unget();
                        return Token{ TokenType::END_SCOPE };
                    }
                    break;
                }
                beggining_of_line = true;
                return Token{ TokenType::NEWLINE };

            case ' ':
                if (auto token = tokenize_space()) {
                    return *token;
                }
                break;

            case ':':
                beggining_of_line = false;
                switch (source.get()) {
                    case '=':
                        return Token{ TokenType::ASSIGN_LIST };

                    default:
                        source.unget();
                        return Token{ TokenType::COLON };
                }

            case '=':
                beggining_of_line = false;
                return Token{ TokenType::ASSIGN };

            case '|':
                beggining_of_line = false;
                switch (source.get()) {
                    case '@':
                        return Token{ TokenType::VALIDATION_DEPENDENCY };

                    case '|':
                        return Token{ TokenType::ORDER_DEPENDENCY };

                    default:
                        source.unget();
                        return Token{ TokenType::IMPLICIT_DEPENDENCY };
                }

            case '$':
                beggining_of_line = false;
                return tokenize_dollar();

            default:
                beggining_of_line = false;
                return tokenize_word(c);
        }
    }
}

void Tokenizer::skip_space() {
    while (true) {
        const auto token = get_next();
        if (token.type != TokenType::SPACE) {
            unget(token);
            return;
        }
    }
}

void Tokenizer::skip_whitespace() {
    while (true) {
        const auto token = get_next();
        if (token.type != TokenType::SPACE && token.type != TokenType::NEWLINE) {
            unget(token);
            return;
        }
    }
}

void Tokenizer::unget(const Token& token) {
    if (ungot) {
        throw Exception("double unget");
    }
    ungot = token;
}

Tokenizer::CharacterType Tokenizer::type(int c) {
    const auto it = special_characters.find(c);
    if (it != special_characters.end()) {
        return it->second;
    }
    else if ((c >= 'a') && (c <= 'z') || (c >= 'A') && (c <= 'Z') || (c >= '0' && c <= '9')) {
        return CharacterType::SIMPLE_VARIABLE;
    }
    else {
        return CharacterType::OTHER;
    }
}

bool Tokenizer::is_braced_variable(std::string name) {
    return std::all_of(name.begin(), name.end(), [](char c) { return is_braced_variable(c); });
}

bool Tokenizer::is_simple_variable(std::string name) {
    return std::all_of(name.begin(), name.end(), [](char c) { return is_simple_variable(c); });
}

Tokenizer::Token Tokenizer::tokenize_braced_variable() {
    std::string name;

    while (true) {
        auto c = source.get();

        if (c == EOF || c == '\n') {
            source.unget();
            throw Exception("unclosed '${'");
        }
        if (is_braced_variable(c)) {
            name += static_cast<char>(c);
        }
        else if (c == '}') {
            return Token{ TokenType::VARIABLE_REFERENCE, name };
        }
        else {
            while ((c = source.get()) != EOF && c != '}' && c != '\n') {
            }
            if (c != '}') {
                source.unget();
            }
            throw Exception("invalid character in variable name");
        }
    }
}

Tokenizer::Token Tokenizer::tokenize_dollar() {
    switch (const int c = source.get()) {
        case '\n': {
            while (source.get() == ' ') {
            }
            source.unget();
            return tokenize_word('\n');
        }

        case '$':
        case ' ':
            return tokenize_word(c);

        case '{':
            return tokenize_braced_variable();

        default:
            return tokenize_variable(c);
    }
}

std::optional<Tokenizer::Token> Tokenizer::tokenize_space() {
    auto length = 1;

    while (source.get() == ' ') {
        length += 1;
    }
    source.unget();

    if (beggining_of_line) {
        if (length == indent) {
            return {};
        }
        else {
            const auto type = length > indent ? TokenType::BEGIN_SCOPE : TokenType::END_SCOPE;
            beggining_of_line = false;
            indent = length;
            return Token{ type };
        }
    }
    else {
        return Token{ TokenType::SPACE, std::string(length, ' ') };
    }
}

Tokenizer::Token Tokenizer::tokenize_word(int first_character) {
    std::string value{ static_cast<char>(first_character) };

    while (true) {
        const int c = source.get();

        if (is_word(c)) {
            value += static_cast<char>(c);
        }
        else {
            source.unget();
            break;
        }
    }

    const auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token{ it->second };
    }
    else {
        return Token{ TokenType::WORD, value };
    }
}

Tokenizer::Token Tokenizer::tokenize_variable(int c) {
    std::string name;

    while (is_simple_variable(c)) {
        name += static_cast<char>(c);
        c = source.get();
    }
    source.unget();
    if (name.empty()) {
        throw Exception("empty variable name");
    }
    return Token{ TokenType::VARIABLE_REFERENCE, name };
}
