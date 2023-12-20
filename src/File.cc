/*
File.cc --

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

#include "File.h"

#include "../foundation/lib/Util.h"
#include "Tokenizer.h"
#include <Exception.h>
#include <iostream>

File::File(const std::filesystem::path& filename, const File* previous) : source_filename{ filename }, build_filename{ replace_extension(filename, "ninja") }, previous{ previous } {
    if (previous) {
        // TODO: derive build directory relavtie to previous
    }
    else {
        build_directory = ".";
    }
    source_directory = filename.parent_path();

    parse(filename);

    for (const auto& subninja:subninjas) {
        auto name = subninja.string();

        // TODO: derive build directory
        subfiles.emplace_back(source_directory / name, this);
    }
}

void File::process() {
    for (auto& build : builds) {
        build.process_outputs(*this);
        auto current_outputs = build.get_outputs();
        outputs.insert(current_outputs.begin(), current_outputs.end());
    }

    for (auto& pair : bindings) {
        pair.second.process(*this);
    }

    for (auto& pair : rules) {
        pair.second.process(*this);
    }

    for (auto& build : builds) {
        build.process(*this);
    }
}

const Rule* File::find_rule(const std::string& name) const {
    for (auto file = this; file; file = file->previous) {
        const auto& it = file->rules.find(name);

        if (it != rules.end()) {
            return &it->second;
        }
    }

    return nullptr;
}

const Variable* File::find_variable(const std::string& name) const {
    for (auto file = this; file; file = file->previous) {
        const auto& it = file->bindings.find(name);

        if (it != bindings.end()) {
            return &it->second;
        }
    }

    return nullptr;
}

void File::create_output() const {
    auto stream = std::ofstream(build_filename);

    if (stream.fail()) {
        throw Exception("can't create output '%s'", build_filename.c_str());
    }

    // TODO: print header (automatically created)

    bindings.print(stream, "");

    for (auto& pair : rules) {
        pair.second.print(stream);
    }

    for (auto& build: builds) {
        build.print(stream);
    }
}

void File::parse(const std::filesystem::path& filename) {
    auto tokenizer = Tokenizer(filename);

    try {
        while (const auto token = tokenizer.next()) {
            switch (token.type) {
                case Tokenizer::TokenType::NEWLINE:
                case Tokenizer::TokenType::SPACE:
                case Tokenizer::TokenType::END:
                    break;

                case Tokenizer::TokenType::BUILD:
                    parse_build(tokenizer);
                    break;

                case Tokenizer::TokenType::DEFAULT:
                    parse_default(tokenizer);
                    break;

                case Tokenizer::TokenType::INCLUDE:
                    // TODO
                    break;

                case Tokenizer::TokenType::POOL:
                    parse_pool(tokenizer);
                    break;

                case Tokenizer::TokenType::RULE:
                    parse_rule(tokenizer);
                    break;

                case Tokenizer::TokenType::SUBNINJA:
                    parse_subninja(tokenizer);
                    break;

                case Tokenizer::TokenType::WORD:
                    parse_assignment(tokenizer, token.value);
                    break;

                case Tokenizer::TokenType::ASSIGN:
                case Tokenizer::TokenType::ASSIGN_LIST:
                case Tokenizer::TokenType::BEGIN_SCOPE:
                case Tokenizer::TokenType::COLON:
                case Tokenizer::TokenType::END_SCOPE:
                case Tokenizer::TokenType::IMPLICIT_DEPENDENCY:
                case Tokenizer::TokenType::ORDER_DEPENDENCY:
                case Tokenizer::TokenType::VALIDATION_DEPENDENCY:
                case Tokenizer::TokenType::VARIABLE_REFERENCE:
                    throw Exception("invalid token");
            }
        }
    } catch (Exception& ex) {
        std::cerr << filename.string() << ":" << tokenizer.current_line_number() << ": " << ex.what() << std::endl;
        throw Exception();
    }
}

void File::parse_assignment(Tokenizer& tokenizer, const std::string& variable_name) {
    const auto token = tokenizer.next(Tokenizer::Skip::SPACE);

    auto is_list = false;

    if (token.type == Tokenizer::TokenType::ASSIGN_LIST) {
        is_list = true;
    }
    else if (token.type != Tokenizer::TokenType::ASSIGN) {
        throw Exception("invalid assignment");
    }

    bindings.add(variable_name, Variable{ variable_name, is_list, tokenizer });
}

void File::parse_build(Tokenizer& tokenizer) { builds.emplace_back(tokenizer); }

void File::parse_default(Tokenizer& tokenizer) {
    throw Exception("default not implemented yet");
    // TODO: implement
}

void File::parse_pool(Tokenizer& tokenizer) {
    throw Exception("pool not implemented yet");
    // TODO: implement
}

void File::parse_rule(Tokenizer& tokenizer) {
    tokenizer.skip_space();
    const auto token = tokenizer.next();
    if (token.type != Tokenizer::TokenType::WORD) {
        throw Exception("name expected");
    }
    rules[token.string()] = Rule(token.string(), tokenizer);
}

void File::parse_subninja(Tokenizer& tokenizer) {
    subninjas.emplace_back(tokenizer, Tokenizer::TokenType::NEWLINE);
}
