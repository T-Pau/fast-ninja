/*
Text.h --

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

#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <unordered_set>
#include <vector>

#include "Tokenizer.h"

class File;
class Variable;

/*
 element types:
 - text -> print literally
 - unresolved variable -> error on print
 - resolved scalar variable -> print as $name or ${complex.name}
 - resolved list variable -> expand on print
 - source filename -> print as $source_dir/name
 - build filename -> print as $build_dir/name
 */
class Text {
  public:
    enum class ElementType {
        BUILD_FILE,
        SOURCE_FILE,
        VARIABLE,
        WHITESPACE,
        WORD
    };
    class Element {
      public:
        Element(ElementType type, std::string value) : type{type}, value{ std::move(value) } {}

        [[nodiscard]] bool is_build_file() const {return type == ElementType::BUILD_FILE;}
        [[nodiscard]] bool is_file() const {return is_build_file() || is_source_file();}
        [[nodiscard]] bool is_source_file() const {return type == ElementType::SOURCE_FILE;}
        [[nodiscard]] bool is_variable() const {return type == ElementType::VARIABLE;}
        [[nodiscard]] bool is_word() const {return type == ElementType::WORD;}
        [[nodiscard]] std::string string() const;

        ElementType type;
        std::string value;
        const Variable *variable = nullptr;
    };

    Text() = default;
    Text(Tokenizer& tokenizer, Tokenizer::TokenType terminator);
    explicit Text(std::string value): Text{std::vector<Element>{Element{ElementType::WORD, std::move(value)}}} {}
    explicit Text(std::vector<Element> elements): elements{std::move(elements)} {}

    void append(const Text& other) {elements.insert(elements.end(), other.elements.begin(), other.elements.end());}
    void emplace_back(ElementType type, std::string value) { elements.emplace_back(type, std::move(value)); }

    void print(std::ostream& stream) const;
    void process(const File& file);
    void collect_words(std::unordered_set<std::string>& words) const;
    [[nodiscard]] bool empty() const {return elements.empty();}
    [[nodiscard]] std::string string() const;

    [[nodiscard]] std::vector<Element>::iterator begin() { return elements.begin(); }

    [[nodiscard]] std::vector<Element>::iterator end() { return elements.end(); }

    [[nodiscard]] std::vector<Element>::const_iterator begin() const { return elements.begin(); }

    [[nodiscard]] std::vector<Element>::const_iterator end() const { return elements.end(); }

    Element& operator[](size_t index) { return elements[index]; }

    const Element& operator[](size_t index) const { return elements[index]; }

  private:
    std::vector<Element> elements;
};

std::ostream& operator<<(std::ostream& stream, const Text& text);

#endif // TEXT_H
