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

#include "Exception.h"

// clang-format off
std::unordered_map<std::string, Tokenizer::TokenType> Tokenizer::keywords = {
    {"build", TokenType::BUILD},
    {"built-files-list", TokenType::BUILT_FILES},
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
    {'#', CharacterType::COMMENT},
    {'$', CharacterType::PUNCTUATION},
    {'-', CharacterType::SIMPLE_VARIABLE},
    {'.', CharacterType::BRACED_VARIABLE},
    {':', CharacterType::PUNCTUATION},
    {'=', CharacterType::PUNCTUATION},
    {'@', CharacterType::PUNCTUATION},
    {'_', CharacterType::SIMPLE_VARIABLE},
    {'|', CharacterType::PUNCTUATION},
    {'{', CharacterType::PUNCTUATION},
    {'}', CharacterType::PUNCTUATION}
};
// clang-format on

Tokenizer::Tokenizer(const std::filesystem::path& filename) : filename{ filename }, source{Symbol( filename) } {
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

        case TokenType::BEGIN_FILENAME:
            return "{{";

        case TokenType::BUILD:
            return "build";

        case TokenType::BUILT_FILES:
            return "built_files";

        case TokenType::COLON:
            return ":";

        case TokenType::DEFAULT:
            return "default";

        case TokenType::END:
            return "END";

        case TokenType::END_FILENAME:
            return "}}";

        case TokenType::END_SCOPE:
            return "}";

        case TokenType::IMPLICIT_DEPENDENCY:
            return "|";

        case TokenType::INCLUDE:
            return "include";

        case TokenType::NEWLINE:
            return "newline";

        case TokenType::ORDER_DEPENDENCY:
            return "||";

        case TokenType::POOL:
            return "pool";

        case TokenType::RULE:
            return "rule";

        case TokenType::SPACE:
            return "space";

        case TokenType::SUBNINJA:
            return "subninja";

        case TokenType::VALIDATION_DEPENDENCY:
            return "|@";

        case TokenType::VARIABLE_REFERENCE:
            return "$";

        case TokenType::WORD:
            return "word";
    }

    throw Exception("invalid token type");
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

    while (true) {
        auto location = source.location();

        if (begining_of_line) {
            begining_of_line = false;
            auto new_indent = count_space();
            // TODO: doesn't return space token for indent, is that okay?
            if (new_indent != indent) {
                const auto type = new_indent > indent ? TokenType::BEGIN_SCOPE : TokenType::END_SCOPE;
                indent = new_indent;
                source.expand_location(location);
                return Token{location, type};
            }
        }

        auto c = next_character();
        switch (c.type) {
            case CharacterType::COMMENT:
                while (!(c = next_character()).is_end_of_line()) {
                }
                if (c.is_end()) {
                    location = source.location();
                    return Token{location, TokenType::END};
                }
                else {
                    unget_character(c);
                    continue;
                }

            case CharacterType::PUNCTUATION:
                switch (c.value) {
                    case ':': {
                        begining_of_line = false;
                        auto c2 = next_character();
                        if (c2.is_equal()) {
                            source.expand_location(location);
                            return Token{location, TokenType::ASSIGN_LIST};
                        }
                        else {
                            unget_character(c2);
                            return Token{location, TokenType::COLON};
                        }
                    }

                    case '=':
                        begining_of_line = false;
                        return Token{location, TokenType::ASSIGN};

                    case '|': {
                        begining_of_line = false;
                        auto c2 = next_character();
                        switch (c2.value) {
                            case '@':
                                source.expand_location(location);
                                return Token{location, TokenType::VALIDATION_DEPENDENCY};

                            case '|':
                                source.expand_location(location);
                                return Token{location, TokenType::ORDER_DEPENDENCY};

                            default:
                                unget_character(c2);
                                return Token{location, TokenType::IMPLICIT_DEPENDENCY};
                        }
                    }

                    case '{':
                    case '}': {
                        begining_of_line = false;
                        auto c2 = next_character();
                        if (c2 == c) {
                            source.expand_location(location);
                            return Token{location, c.is_brace_open() ? TokenType::BEGIN_FILENAME : TokenType::END_FILENAME};
                        }
                        else {
                            unget_character(c2);
                            return tokenize_word(location, c);
                        }
                    }

                    case '$':
                        begining_of_line = false;
                        return tokenize_dollar(location);

                    default:
                        return tokenize_word(location, c);
                }

            case CharacterType::END:
                return Token{location, TokenType::END};

            case CharacterType::ILLEGAL:
                throw Exception{"tabs are not allowed, use spaces"};

            case CharacterType::NEWLINE:
                if (begining_of_line) {
                    if (indent > 0) {
                        indent = 0;
                        source.unget();
                        return Token{location, TokenType::END_SCOPE};
                    }
                    break;
                }
                begining_of_line = true;
                return Token{location, TokenType::NEWLINE};

            case CharacterType::SPACE:
                return tokenize_space(location);


            default:
                begining_of_line = false;
                return tokenize_word(location, c);
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


Tokenizer::Token Tokenizer::tokenize_braced_variable(Location location) {
    std::string name;

    while (true) {
        auto c = next_character();

        if (c.is_end_of_line()) {
            unget_character(c);
            throw Exception("unclosed '${'");
        }
        if (c.is_braced_variable()) {
            name += c.character();
        }
        else if (c.is_brace_close()) {
            source.expand_location(location);
            return Token{location, TokenType::VARIABLE_REFERENCE, name};
        }
        else {
            auto c2 = next_character();
            while (!c2.is_end_of_line() && !c2.is_brace_close()) {
                c2 = next_character();
            }
            if (!c2.is_brace_close()) {
                unget_character(c2);
            }
            throw Exception("invalid character '%c' in variable name", c.value);
        }
    }
}

Tokenizer::Token Tokenizer::tokenize_dollar(Location location) {
    auto c = next_character();
    if (c.is_brace_open()) {
        return tokenize_braced_variable(location);
    }
    else {
        return tokenize_variable(location, c);
    }
}

int Tokenizer::count_space() {
    auto length = 0;

    auto c = next_character();
    while (c.is_space()) {
        length += 1;
        c = next_character();
    }
    unget_character(c);

    return length;
}

Tokenizer::Token Tokenizer::tokenize_space(Location location) {
    auto spaces = std::string(count_space() + 1, ' ');
    source.expand_location(location);
    return Token{location, TokenType::SPACE, spaces};
}


Tokenizer::Token Tokenizer::tokenize_word(Location location, Character first_character) {
    std::string value{first_character.character()};

    while (true) {
        auto c = next_character();

        if (c.is_word()) {
            value += c.character();
        }
        else {
            unget_character(c);
            break;
        }
    }
    source.expand_location(location);

    const auto it = keywords.find(value);
    if (it != keywords.end()) {
        return Token{location, it->second};
    }
    else {
        return Token{location, TokenType::WORD, value};
    }
}


Tokenizer::Token Tokenizer::tokenize_variable(Location location, Character c) {
    std::string name;

    while (c.is_simple_variable()) {
        name += c.character();
        c = next_character();
    }
    unget_character(c);
    if (name.empty()) {
        throw Exception("empty variable name");
    }
    source.expand_location(location);
    return Token{location, TokenType::VARIABLE_REFERENCE, name};
}


Tokenizer::Character Tokenizer::next_character() {
    if (ungot_character) {
        auto c = *ungot_character;
        ungot_character.reset();
        return c;
    }

    auto c = source.get();
    if (c == '$') {
        auto c2 = source.get();
        if (c2 == ' ' || c2 == '$' || c2 == '\n' || c2 == ':') {
            return {CharacterType::SIMPLE_VARIABLE, c2};
        }
        else {
            source.unget();
            return Character{c};
        }
    }
    else {
        return Character{c};
    }
}


void Tokenizer::unget_character(Character c) {
    if (ungot_character) {
        throw Exception("double unget");
    }
    ungot_character = c;
}


 Tokenizer::Character::Character(int value): value(value) {
    auto it = special_characters.find(value);
    if (it != special_characters.end()) {
        type = it->second;
    }
    else if ((value >= 'a') && (value <= 'z') || (value >= 'A') && (value <= 'Z') || (value >= '0' && value <= '9')) {
        type = CharacterType::SIMPLE_VARIABLE;
    }
    else {
        type = CharacterType::OTHER;
    }
}
