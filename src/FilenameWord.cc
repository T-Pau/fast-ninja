/*
ExplicitFilename.cc -- 

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

#include "FilenameWord.h"

#include <Exception.h>

#include "FilenameVariable.h"


FilenameWord::FilenameWord(Tokenizer& tokenizer, bool force_build): force_build{force_build} {
    std::string string;

    auto first = true;
    auto braced = false;

    tokenizer.skip_space();

    while (auto token = tokenizer.next()) {
        if (token.type == Tokenizer::TokenType::SPACE && !braced) {
            break;
        }
        else if (token.type == Tokenizer::TokenType::END_FILENAME) {
            if (braced) {
                break;
            }
            else {
                throw Exception("unmatched }}");
            }
        }
        else if (token.type == Tokenizer::TokenType::BEGIN_FILENAME) {
            if (!braced && first) {
                first = false;
                braced = true;
                continue;
            }
            else {
                throw Exception("cannot nest filenames");
            }
        }
        else if (token.type == Tokenizer::TokenType::NEWLINE) {
            if (!braced) {
                tokenizer.unget(token);
                break;
            }
            throw Exception("unterminated filename");
        }

        first = false;

        if (token.is_variable_refrence()) {
            if (!string.empty()) {
                elements.emplace_back(string);
                string = "";
            }
            elements.emplace_back(VariableReference(token.value));
            resolved = false;
        }
        else if (braced || token.type == Tokenizer::TokenType::WORD) {
            string += token.value;
        }
        else {
            tokenizer.unget(token);
            break;
        }
    }

    if (!string.empty()) {
        elements.emplace_back(string);
    }
}

void FilenameWord::resolve(const ResolveContext& context) {
    auto contains_filename_variable = false;
    resolved = true;

    for (auto& element : elements) {
        if (std::holds_alternative<VariableReference>(element)) {
            auto& variable_reference = std::get<VariableReference>(element);
            if (auto variable = variable_reference.resolve(context)) {
                if (variable->is_filename()) {
                    contains_filename_variable = true;
                }
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

    // TODO: this shouldn't be neccessary
    if (resolved && !contains_filename_variable) {
        auto name = std::string();
        for (const auto& element: elements) {
            if (std::holds_alternative<std::string>(element)) {
                name += std::get<std::string>(element);
            }
            else if (std::holds_alternative<const Variable*>(element)) {
                name += std::get<const Variable*>(element)->string();
            }
        }
        filename = Filename{force_build ? Filename::Type::BUILD : Filename::Type::UNKNOWN, name};
        filename->resolve(context);
    }
}

void FilenameWord::collect_filenames(std::vector<Filename>& filenames) const {
    if (filename) {
        filenames.emplace_back(*filename);
    }
    else {
        std::string prefix;
        std::string postfix;
        auto current_string = &prefix;
        const FilenameVariable* filename_variable{};

        for (auto& element : elements) {
            if (std::holds_alternative<std::string>(element)) {
                *current_string += std::get<std::string>(element);
            }
            else if (std::holds_alternative<VariableReference>(element)) {
                auto variable_reference = std::get<VariableReference>(element);
                throw Exception("unresolved variable %s", variable_reference.name.c_str());
            }
            else {
                auto variable = std::get<const Variable*>(element);
                if (variable->is_filename()) {
                    if (filename_variable) {
                        throw Exception("multiple filename variables in filename not allowed");
                    }
                    else {
                        filename_variable = variable->as_filename();
                        current_string = &postfix;
                    }
                }
                else {
                    *current_string += variable->string();
                }
            }
        }

        if (filename_variable) {
            if (prefix.empty() && postfix.empty()) {
                filename_variable->collect_filenames(filenames);
            }
            else {
                std::vector<Filename> inner_filenames;
                filename_variable->collect_filenames(inner_filenames);
                for (auto& filename : inner_filenames) {
                    filename.name = prefix + filename.name + postfix;
                }
                filenames.insert(filenames.end(), inner_filenames.begin(), inner_filenames.end());
            }
        }
        else {
            filenames.emplace_back(prefix);
        }
    }
}
