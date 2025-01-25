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

#include <FileSource.h>
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
        COMMENT,
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
        BEGIN_FILENAME,
        BEGIN_SCOPE,
        BUILD,
        BUILT_FILES,
        COLON,
        DEFAULT,
        END,
        END_FILENAME,
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

    class Character {
    public:
        explicit Character(int value);
        Character(CharacterType type, int value): type(type), value(value) {}

        [[nodiscard]] char character() const {return static_cast<char>(value);}
        [[nodiscard]] bool is_brace_close() const {return type == CharacterType::PUNCTUATION && value == '}';}
        [[nodiscard]] bool is_brace_open() const {return type == CharacterType::PUNCTUATION && value == '{';}
        [[nodiscard]] bool is_braced_variable() const {return type == CharacterType::SIMPLE_VARIABLE || type == CharacterType::BRACED_VARIABLE;}
        [[nodiscard]] bool is_end() const {return type == CharacterType::END;}
        [[nodiscard]] bool is_end_of_line() const {return type == CharacterType::NEWLINE || type == CharacterType::END;}
        [[nodiscard]] bool is_equal() const {return type == CharacterType::PUNCTUATION && value == '=';}
        [[nodiscard]] bool is_simple_variable() const {return type == CharacterType::SIMPLE_VARIABLE;}
        [[nodiscard]] bool is_space() const {return type == CharacterType::SPACE;}
        [[nodiscard]] bool is_word() const {return type == CharacterType::BRACED_VARIABLE || type == CharacterType::SIMPLE_VARIABLE || type == CharacterType::OTHER;}

        bool operator==(const Character &other) const {return type == other.type && value == other.value;}

        CharacterType type;
        int value;
    };

    class Token {
    public:
        explicit Token(TokenType type) : type{type} {}
        Token(const Location& location, TokenType type) : location{location}, type{type} {}
        Token(const Location& location, TokenType type, std::string value): location{location}, type{type}, value{std::move(value)} {}

        explicit operator bool() const {return type != TokenType::END;}
        [[nodiscard]] bool is_variable_refrence() const {return type == TokenType::VARIABLE_REFERENCE;}
        [[nodiscard]] bool is_whitespace() const {return type == TokenType::SPACE || type == TokenType::NEWLINE;}
        [[nodiscard]] std::string string() const;
        [[nodiscard]] std::string type_name() const {return type_name(type);}
        [[nodiscard]] static std::string type_name(TokenType type);

        Location location;
        TokenType type;
        std::string value;
    };

    Token expect(TokenType type, Skip skip = Skip::NONE);
    [[nodiscard]] Token next(Skip skip = Skip::NONE);
    void skip_space();
    void skip_whitespace();
    void unget(const Token& token);
    [[nodiscard]] const std::filesystem::path& file_name() const {return filename;}

private:
    [[nodiscard]] Character next_character();
    void unget_character(Character c);
    [[nodiscard]] Token get_next();
    [[nodiscard]] int count_space();
    [[nodiscard]] Token tokenize_braced_variable(Location location);
    [[nodiscard]] Token tokenize_dollar(Location location);
    [[nodiscard]] Token tokenize_space(Location location);
    [[nodiscard]] Token tokenize_variable(Location location, Character first_character);
    [[nodiscard]] Token tokenize_word(Location location, Character first_character);

    static std::unordered_map<std::string, TokenType> keywords;
    static std::unordered_map<int, CharacterType> special_characters;

    std::filesystem::path filename;
    FileSource source;
    std::optional<Character> ungot_character;
    std::optional<Token> ungot;
    bool begining_of_line = true;
    int indent = 0;
};

#endif //TOKENIZER_H
