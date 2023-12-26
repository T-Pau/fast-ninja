/*
Tokenizer.h --

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

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>

class Tokenizer {
public:
    explicit Tokenizer(const std::filesystem::path& filename);

    enum class Skip {
        NONE,
        SPACE,
        WHITESPACE
    };

    enum class CharacterType {
        BRACED_VARIABLE,
        END,
        NEWLINE,
        ILLEGAL,
        OTHER,
        PUNCTUATION,
        SIMPLE_VARIABLE,
        SPACE
    };
    enum class TokenType {
        ASSIGN,
        ASSIGN_LIST,
        BEGIN_SCOPE,
        BUILD,
        COLON,
        DEFAULT,
        END,
        END_SCOPE,
        IMPLICIT_DEPENDENCY,
        INCLUDE,
        NEWLINE,
        ORDER_DEPENDENCY,
        POOL,
        RULE,
        SPACE,
        SUBNINJA,
        VALIDATION_DEPENDENCY,
        VARIABLE_REFERENCE,
        WORD
    };

    class Token {
    public:
        explicit Token(TokenType type) : type{type} {}
        Token(TokenType type, std::string value) : type{type}, value{std::move(value)} {}

        explicit operator bool() const {return type != TokenType::END;}
        [[nodiscard]] bool is_whitespace() const {return type == TokenType::SPACE || type == TokenType::NEWLINE;}
        [[nodiscard]] std::string string() const;
        [[nodiscard]] std::string type_name() const {return type_name(type);}
        [[nodiscard]] static std::string type_name(TokenType type);

        TokenType type;
        std::string value;
    };

    Token expect(TokenType type, Skip skip = Skip::NONE);
    [[nodiscard]] Token next(Skip skip = Skip::NONE);
    void skip_space();
    void skip_whitespace();
    void unget(const Token& token);
    [[nodiscard]] int current_line_number() const {return line_number;}

private:
    static CharacterType type(int c);
    static bool is_braced_variable(int c) {const auto ctype = type(c); return ctype == CharacterType::SIMPLE_VARIABLE || ctype == CharacterType::BRACED_VARIABLE;}
    static bool is_braced_variable(std::string name);
    static bool is_simple_variable(int c) {return type(c) == CharacterType::SIMPLE_VARIABLE;}
    static bool is_simple_variable(std::string name);
    static bool is_word(int c) {const auto ctype = type(c); return ctype == CharacterType::BRACED_VARIABLE || ctype == CharacterType::SIMPLE_VARIABLE || ctype == CharacterType::OTHER;}

    [[nodiscard]] Token get_next();
    [[nodiscard]] Token tokenize_braced_variable();
    [[nodiscard]] Token tokenize_dollar();
    [[nodiscard]] std::optional<Token> tokenize_space();
    [[nodiscard]] Token tokenize_variable(int first_character);
    [[nodiscard]] Token tokenize_word(int first_character);

    static std::unordered_map<std::string, TokenType> keywords;
    static std::unordered_map<int, CharacterType> special_characters;

    std::ifstream source;
    std::optional<Token> ungot;
    bool beggining_of_line = true;
    int indent = 0;
    int line_number = 1;
};

#endif //TOKENIZER_H
