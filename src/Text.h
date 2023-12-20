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

#include "Tokenizer.h"
#include <unordered_set>

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
    class Element {
      public:
        Element(std::string value, bool is_variable) : value{ std::move(value) }, is_variable{ is_variable } {}

        std::string value;
        bool is_variable;
        const Variable *variable;
    };

    Text() = default;
    Text(Tokenizer& tokenizer, Tokenizer::TokenType terminator);

    void emplace_back(std::string value, bool is_variable = false) { elements.emplace_back(std::move(value), is_variable); }

    void print(std::ostream& stream) const;
    void process(const File& file);
    void collect_words(std::unordered_set<std::string>& words) const;
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
