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

#include <iostream>

#include "../foundation/lib/Util.h"
#include "Exception.h"
#include "Tokenizer.h"

File::File(const std::filesystem::path& filename, const std::filesystem::path& build_directory, const File* previous) : source_filename{ filename }, previous{ previous }, build_directory{ build_directory.lexically_normal() } {
    source_directory = filename.parent_path();
    build_filename = replace_extension(build_directory / source_filename.filename(), "ninja");

    parse(filename);

    for (const auto& subninja : subninjas) {
        subfiles.emplace_back(std::make_unique<File>(source_directory / subninja, build_directory / std::filesystem::path(subninja).parent_path(), this));
    }
}

void File::process() {
    if (is_top()) {
        rules["fast-ninja"] = Rule("fast-ninja", Bindings({
            Variable("command", false, Text{std::vector<Text::Element>{
                    Text::Element{Text::ElementType::WORD, "fast-ninja"},
                    Text::Element{Text::ElementType::WHITESPACE, " "},
                    Text::Element{Text::ElementType::VARIABLE, "top_source_directory"}
                }}),
            Variable("generator", false, Text("1"))
        }));
        auto outputs = Text{};
        auto inputs = Text{};
        add_generator_build(outputs, inputs);

        builds.emplace_back("fast-ninja", outputs, inputs, Bindings{});
    }

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

    defaults.process(*this);

    for (const auto& file : subfiles) {
        file->process();
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

        if (it != file->bindings.end()) {
            return &it->second;
        }
    }

    return nullptr;
}

void File::create_output() const {
    std::filesystem::create_directories(build_directory);
    auto stream = std::ofstream(build_filename);

    if (stream.fail()) {
        throw Exception("can't create output '%s'", build_filename.c_str());
    }

    auto top_file = this;
    while (!top_file->is_top()) {
        top_file = top_file->previous;
    }

    stream << "top_source_directory = " << top_file->source_directory.string() << std::endl;
    stream << "source_directory = " << source_directory.string() << std::endl;
    stream << "top_build_directory = " << top_file->build_directory.string() << std::endl;
    stream << "build_directory = " << build_directory.string() << std::endl;

    if (!bindings.empty()) {
        stream << std::endl;
        bindings.print(stream, "");
    }

    for (auto& pair : rules) {
        pair.second.print(stream);
    }

    for (auto& build : builds) {
        build.print(stream);
    }

    if (!defaults.empty()) {
        stream << std::endl;
        stream << "default " << defaults << std::endl;
    }

    if (!subninjas.empty()) {
        stream << std::endl;
        for (auto& subninja : subninjas) {
            stream << "subninja " << replace_extension(subninja, "ninja").string() << std::endl;
        }
    }

    for (auto& subfile : subfiles) {
        subfile->create_output();
    }
}

void File::parse(const std::filesystem::path& filename) {
    auto tokenizers = std::vector<Tokenizer>{};
    tokenizers.emplace_back(filename);

    while (!tokenizers.empty()) {
        auto& tokenizer = tokenizers.back();

        try {
            const auto token = tokenizer.next();

            switch (token.type) {
                case Tokenizer::TokenType::END:
                    tokenizers.pop_back();
                break;

                case Tokenizer::TokenType::NEWLINE:
                case Tokenizer::TokenType::SPACE:
                    break;

                case Tokenizer::TokenType::BUILD:
                    parse_build(tokenizer);
                break;

                case Tokenizer::TokenType::DEFAULT:
                    parse_default(tokenizer);
                break;

                case Tokenizer::TokenType::INCLUDE: {
                    auto name = tokenizer.expect(Tokenizer::TokenType::WORD, Tokenizer::Skip::SPACE);
                    includes.insert(name.string());
                    tokenizers.emplace_back(name.string());
                    break;
                }

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
        } catch (Exception& ex) {
            std::cerr << tokenizer.file_name().string() << ":" << tokenizer.current_line_number() << ": " << ex.what() << std::endl;
            throw Exception();
        }
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

void File::parse_default(Tokenizer& tokenizer) { defaults.append(Text(tokenizer, Tokenizer::TokenType::NEWLINE)); }

void File::parse_pool(Tokenizer& tokenizer) {
    tokenizer.skip_space();
    const auto token = tokenizer.next();
    if (token.type != Tokenizer::TokenType::WORD) {
        throw Exception("name expected");
    }
    pools[token.string()] = Pool(token.string(), tokenizer);
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
    auto text = Text{ tokenizer, Tokenizer::TokenType::NEWLINE };

    subninjas.emplace_back(text.string());
}

void File::add_generator_build(Text& outputs, Text& inputs) const {
    if (!outputs.empty()) {
        outputs.emplace_back(Text::ElementType::WHITESPACE, " ");
    }
    outputs.emplace_back(Text::ElementType::WORD, build_filename.string());
    if (!inputs.empty()) {
        inputs.emplace_back(Text::ElementType::WHITESPACE, " ");
    }
    inputs.emplace_back(Text::ElementType::WORD, source_filename.string());
    for (auto& file: includes) {
        inputs.emplace_back(Text::ElementType::WHITESPACE, " ");
        inputs.emplace_back(Text::ElementType::WORD, file);
    }
    for (const auto& file: subfiles) {
        file->add_generator_build(outputs, inputs);
    }
}
