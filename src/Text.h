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
#include <vector>

#include "Tokenizer.h"
#include "Word.h"

class ResolveContext;
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
    Text() = default;
    explicit Text(Tokenizer& tokenizer);
    explicit Text(std::string value, bool escape): Text{std::vector<Word>{Word{std::move(value), escape}}} {}
    explicit Text(std::vector<Word> elements): words{std::move(elements)} {}

    void append(const Text& other) {words.insert(words.end(), other.words.begin(), other.words.end());}
    void emplace_back(const Word& element) { words.emplace_back(element); }

    void print(std::ostream& stream) const;
    void resolve(const ResolveContext& scope);
    [[nodiscard]] bool empty() const {return words.empty();}
    [[nodiscard]] std::string string() const;
    [[nodiscard]] bool contains_unknown_file() const {return false;} // TODO
    [[nodiscard]] bool is_resolved() const {return resolved;}

  private:
    std::vector<Word> words;
    bool resolved{true};
};

std::ostream& operator<<(std::ostream& stream, const Text& text);

#endif // TEXT_H
