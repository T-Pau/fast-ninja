/*
Word.cc --

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

#include "Word.h"

#include <sstream>

#include "FilenameVariable.h"
#include "Exception.h"

Word::Word(Tokenizer& tokenizer) {
    std::string string;

    while (auto token = tokenizer.next()) {
        if (token.is_whitespace()) {
            tokenizer.unget(token);
            break;
        }

        if (token.is_variable_refrence()) {
            if (!string.empty()) {
                elements.emplace_back(StringElement{string, true});
                string = "";
            }
            elements.emplace_back(VariableReference(token.value));
            resolved = false;
        }
        else if (token.type == Tokenizer::TokenType::BEGIN_FILENAME) {
            tokenizer.unget(token);
            elements.emplace_back(FilenameWord(tokenizer));
        }
        else {
            string += token.string();
        }
    }

    if (!string.empty()) {
        elements.emplace_back(StringElement{string, true});
    }
}

std::string Word::string() const {
    auto stream = std::stringstream{};

    stream << *this;

    return stream.str();
}

void Word::print(std::ostream& stream) const {
    std::string prefix;
    std::string postfix;
    std::string* current_string = &prefix;
    const FilenameVariable* filename_variable{};
    const FilenameWord* filename_word{};

    for (auto& element: elements) {
        if (std::holds_alternative<StringElement>(element)) {
            *current_string += std::get<StringElement>(element).string();
        }
        else if (std::holds_alternative<VariableReference>(element)) {
            auto& variable_reference = std::get<VariableReference>(element);
            *current_string += "$" + variable_reference.name;
        }
        else if (std::holds_alternative<const Variable*>(element)) {
            auto& variable = std::get<const Variable*>(element);
            if (variable->is_text()) {
                    *current_string += variable->string();
            }
            else {
                if (filename_variable || filename_word) {
                    throw Exception("multiple file names in word not allowed");
                }
                filename_variable = variable->as_filename();
                current_string = &postfix;
            }
        }
        else if (std::holds_alternative<FilenameWord>(element)) {
            if (filename_variable || filename_word) {
                throw Exception("multiple file names in word not allowed");
            }
            filename_word = &std::get<FilenameWord>(element);
            current_string = &postfix;
        }
    }

    if (filename_variable || filename_word) {
        std::vector<Filename> filenames;
        if (filename_variable) {
            filename_variable->collect_filenames(filenames);
        }
        else {
            filename_word->collect_filenames(filenames);
        }
        auto first = true;
        for (const auto& filename: filenames) {
            if (first) {
                first = false;
            }
            else {
                stream << " ";
            }
            stream << prefix << filename << postfix;
        }
    }
    else {
        stream << prefix;
    }
}

void Word::resolve(const ResolveContext& context) {
    resolved = true;

    for (auto& element : elements) {
        if (std::holds_alternative<VariableReference>(element)) {
            if (context.expand_variables) {
                const auto& variable_reference = std::get<VariableReference>(element);
                if (auto variable = variable_reference.resolve(context)) {
                    if (!variable->is_resolved()) {
                        context.result.add_unresolved_variable_use(variable->name);
                        resolved = false;
                    }
                    element = variable;
                }
                else {
                    throw Exception("unknown variable %s", variable_reference.name.c_str());
                }
            }
        }
        else if (std::holds_alternative<FilenameWord>(element)) {
            auto& filename = std::get<FilenameWord>(element);
            filename.resolve(context);
        }
    }
}


std::ostream& operator<<(std::ostream& stream, const Word& word) {
    word.print(stream);
    return stream;
}
