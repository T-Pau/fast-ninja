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

#include <algorithm>
#include <iostream>
#include <ranges>

#include "../foundation/lib/Util.h"
#include "Exception.h"
#include "FilenameVariable.h"
#include "TextVariable.h"
#include "Tokenizer.h"
#include <DiagnosticOutput.h>

File::File(const std::filesystem::path& filename, const std::filesystem::path& build_directory, const File* next) : Scope(next), source_filename{ filename }, build_directory{ build_directory.lexically_normal() } {
    source_directory = filename.parent_path();
    build_filename = replace_extension(build_directory / source_filename.filename(), "ninja");

    bindings.add(std::make_shared<FilenameVariable>("build_directory", FilenameList{ Filename{{}, Filename::Type::COMPLETE, build_directory.string()}}));
    bindings.add(std::make_shared<FilenameVariable>("source_directory", FilenameList{ Filename{{}, Filename::Type::COMPLETE, source_directory.string()}}));
    if (is_top()) {
        bindings.add(std::make_shared<FilenameVariable>("top_build_directory", FilenameList{Filename{{}, Filename::Type::COMPLETE, top_file()->build_directory.string()}}));
        bindings.add(std::make_shared<FilenameVariable>("top_source_directory", FilenameList{Filename{{}, Filename::Type::COMPLETE, top_file()->source_directory.string()}}));
    }

    parse(filename);

    for (const auto& subninja : subninjas) {
        subfiles.emplace_back(std::make_unique<File>(source_directory / subninja, build_directory / std::filesystem::path(subninja).parent_path(), this));
    }
}

void File::process() {
    auto generator_bindings = Bindings{};
    generator_bindings.add(std::shared_ptr<Variable>(new TextVariable{ "command", Text{ std::vector<Word>{ Word{ "fast-ninja", false }, Word{ " ", false }, Word{ source_directory.string(), true } } } }));
    generator_bindings.add(std::shared_ptr<Variable>(new TextVariable{ "generator", Text{ "1", false } }));

    rules["fast-ninja"] = Rule(this, "fast-ninja", generator_bindings);
    auto ninja_outputs = std::vector<Filename>{};
    auto ninja_inputs = std::vector<Filename>{};
    add_generator_build(ninja_outputs, ninja_inputs);
    if (built_files_list) {
        ninja_outputs.emplace_back(*built_files_list);
    }

    builds.emplace_back(this, "fast-ninja", Dependencies{ FilenameList{ ninja_outputs } }, Dependencies{ FilenameList{ ninja_inputs } }, Bindings{});

    process_bindings();
    process_output();
    process_rest();
}


void File::process_bindings() { // NOLINT(misc-no-recursion)
    bindings.resolve(*this, true, false);

    for (const auto& file : subfiles) {
        file->process_bindings();
    }
}


void File::process_output() { // NOLINT(misc-no-recursion)
    auto top_file = const_cast<File*>(top()->as_file());
    if (!top_file) {
        throw Exception("internal error: top scope is not a file");
    }

    for (auto& build : builds) {
        build.process_outputs(*this);
        build.collect_output_files(top_file->outputs);
    }

    for (const auto& file : subfiles) {
        file->process_output();
    }
}

void File::process_rest() { // NOLINT(misc-no-recursion)
    bindings.resolve(*this);

    for (auto& rule : std::views::values(rules)) {
        rule.process(*this);
    }

    for (auto& build : builds) {
        build.process(*this);
    }

    ResolveResult result;
    auto context = ResolveContext{ *this, result };
    defaults.resolve(context);
    if (!result.unresolved_used_variables.empty()) {
        // TODO: error: unresolved variables
    }

    for (const auto& file : subfiles) {
        file->process_rest();
    }
}

const Rule* File::find_rule(const std::string& name) const {
    for (auto file = this; file; file = file->next_file()) {
        const auto& it = file->rules.find(name);

        if (it != rules.end()) {
            return &it->second;
        }
    }

    return nullptr;
}

const Variable* File::find_variable(const std::string& name) const {
    for (auto file = this; file; file = file->next_file()) {
        const auto& it = file->bindings.find(name);

        if (it != file->bindings.end()) {
            return it->second.get();
        }
    }

    return nullptr;
}

void File::create_output() const { // NOLINT(misc-no-recursion)
    std::filesystem::create_directories(build_directory);

    {
        auto stream = std::ofstream(build_filename);

        if (stream.fail()) {
            throw Exception("can't create output '%s'", build_filename.c_str());
        }

        stream << "# This file is automatically created by fast-ninja from " << source_filename.generic_string() << std::endl;
        stream << "# Do not edit." << std::endl << std::endl;

        if (!bindings.empty()) {
            bindings.print(stream, "");
        }

        for (auto& rule : std::views::values(rules)) {
            rule.print(stream);
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
                stream << "subninja " << (build_directory / replace_extension(subninja, "ninja")).lexically_normal().string() << std::endl;
            }
        }
    }

    for (auto& subfile : subfiles) {
        subfile->create_output();
    }

    if (built_files_list) {
        auto files = std::vector(outputs.begin(), outputs.end());
        std::ranges::sort(files);
        // TODO: exclude subninja files
        auto stream = std::ofstream(built_files_list->full_name());
        for (const auto& file : files) {
            stream << file << std::endl;
        }
    }
}

void File::parse(const std::filesystem::path& filename) {
    auto tokenizers = std::vector<Tokenizer>{};
    tokenizers.emplace_back(filename);

    while (!tokenizers.empty()) {
        auto& tokenizer = tokenizers.back();

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

            case Tokenizer::TokenType::BUILT_FILES:
                parse_built_files_list(tokenizer);
                break;

            case Tokenizer::TokenType::DEFAULT:
                parse_default(tokenizer);
                break;

            case Tokenizer::TokenType::INCLUDE: {
                auto name = tokenizer.expect(Tokenizer::TokenType::WORD, Tokenizer::Skip::SPACE);
                auto include_filename = Filename(name.location, Filename::Type::SOURCE, name.string());
                auto result = ResolveResult();
                include_filename.resolve(ResolveContext(*this, result));
                includes.insert(include_filename);
                tokenizers.emplace_back(include_filename.full_name().string());
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
            case Tokenizer::TokenType::BEGIN_FILENAME:
            case Tokenizer::TokenType::COLON:
            case Tokenizer::TokenType::END_FILENAME:
            case Tokenizer::TokenType::END_SCOPE:
            case Tokenizer::TokenType::IMPLICIT_DEPENDENCY:
            case Tokenizer::TokenType::ORDER_DEPENDENCY:
            case Tokenizer::TokenType::VALIDATION_DEPENDENCY:
            case Tokenizer::TokenType::VARIABLE_REFERENCE:
                DiagnosticOutput::global.error({}, token.location) << "unexpected " << token.type_name();
                throw Exception();
        }
    }
}

void File::parse_assignment(Tokenizer& tokenizer, const std::string& variable_name) {
    const auto token = tokenizer.next(Tokenizer::Skip::SPACE);

    if (token.type == Tokenizer::TokenType::ASSIGN) {
        bindings.add(std::shared_ptr<Variable>(new TextVariable(variable_name, tokenizer)));
    }
    else if (token.type == Tokenizer::TokenType::ASSIGN_LIST) {
        bindings.add(std::shared_ptr<Variable>(new FilenameVariable(variable_name, tokenizer)));
    }
    else {
        DiagnosticOutput::global.error({}, token.location, "invalid assignment");
        throw Exception();
    }
}


void File::parse_build(Tokenizer& tokenizer) {
    builds.emplace_back(this, tokenizer);
}


void File::parse_built_files_list(Tokenizer& tokenizer) {
    if (!is_top()) {
        // TODO: include location
        DiagnosticOutput::global.error({}, {}, "built_files_list only allowed in top ninja file");
    }
    if (built_files_list) {
        // TODO: include location
        DiagnosticOutput::global.error({}, {}, "built_files_list already specified");
    }
    auto text = Text{tokenizer};
    built_files_list = Filename{text.location(), Filename::Type::BUILD, text.string()};
}


void File::parse_default(Tokenizer& tokenizer) {
    // TODO: append in case of multiple defaults statements
    defaults = FilenameList{ tokenizer, FilenameList::BUILD };
}


void File::parse_pool(Tokenizer& tokenizer) {
    tokenizer.skip_space();
    const auto token = tokenizer.next();
    if (token.type != Tokenizer::TokenType::WORD) {
        DiagnosticOutput::global.error({}, token.location, "name expected");
        throw Exception();
    }
    pools[token.string()] = Pool(token.string(), tokenizer);
}

void File::parse_rule(Tokenizer& tokenizer) {
    tokenizer.skip_space();
    const auto token = tokenizer.next();
    if (token.type != Tokenizer::TokenType::WORD) {
        DiagnosticOutput::global.error({}, token.location, "name expected");
        throw Exception();
    }
    rules[token.string()] = Rule(this, token.string(), tokenizer);
}

void File::parse_subninja(Tokenizer& tokenizer) {
    auto text = Text{ tokenizer };

    subninjas.emplace_back(text.string());
}

void File::add_generator_build(std::vector<Filename>& ninja_outputs, std::vector<Filename>& ninja_inputs) const { // NOLINT(misc-no-recursion)
    ninja_outputs.emplace_back(Location{}, Filename::Type::BUILD, build_filename.string());
    ninja_inputs.insert(ninja_inputs.end(), includes.begin(), includes.end());
    ninja_inputs.emplace_back(Location{}, Filename::Type::COMPLETE, source_filename.string());
    for (const auto& file : subfiles) {
        file->add_generator_build(ninja_outputs, ninja_inputs);
    }
}

const File* File::next_file() const {
    if (!next) {
        return {};
    }

    if (auto file = dynamic_cast<const File*>(next)) {
        return file;
    }
    throw Exception("internal error: file contained in non-file scope");
}
