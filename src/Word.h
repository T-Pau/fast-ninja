#ifndef WORD_H
#define WORD_H
#include <variant>

/*
Word.h --

Copyright (C) Dieter Baron

The authors can be contacted at <accelerate@tpau.group>

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

#include <string>
#include <vector>
#include <unordered_set>

#include "FilenameWord.h"
#include "Filename.h"
#include "VariableReference.h"

class FilenameWord;

class Word {
  public:
    Word(Tokenizer& tokenizer);
    explicit Word(std::string text) {elements.emplace_back(std::move(text));};
    explicit Word(VariableReference variable_reference) {elements.emplace_back(variable_reference);}
    Word() = default;

    [[nodiscard]] bool empty() const { return elements.empty(); }

    [[nodiscard]] std::string string() const;
    void print(std::ostream& stream) const;

    void resolve(const ResolveContext& scope);

  private:
    void append_string(std::string string) { elements.emplace_back(std::move(string)); }
    void append_variable(std::string string) { elements.emplace_back(VariableReference(std::move(string))); }
    void append_filename(FilenameWord filname) {elements.emplace_back(std::move(filname));}

    std::vector<std::variant<std::string, VariableReference, FilenameWord>> elements;
};

std::ostream& operator<<(std::ostream& stream, const Word& word);

#endif // WORD_H
