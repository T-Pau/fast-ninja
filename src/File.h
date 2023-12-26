/*
File.h --

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

#ifndef FILE_H
#define FILE_H

#include "Bindings.h"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Build.h"
#include "Rule.h"
#include "Variable.h"


class Tokenizer;

class File {
  public:
    File() = default;
    explicit File(const std::filesystem::path& filename, const std::filesystem::path& build_directory = ".", const File* previous = nullptr);

    void process();
    [[nodiscard]] bool is_output(const std::string& word) const {return outputs.contains(word);}
    [[nodiscard]] const Rule* find_rule(const std::string& name) const;
    [[nodiscard]] const Variable* find_variable(const std::string& name) const;

    void create_output() const;

    std::filesystem::path source_directory;
    std::filesystem::path build_directory;

  private:
    void parse(const std::filesystem::path& filename);
    void parse_assignment(Tokenizer& tokenizer, const std::string& variable_name);
    void parse_build(Tokenizer& tokenizer);
    void parse_default(Tokenizer& tokenizer);
    void parse_pool(Tokenizer& tokenizer);
    void parse_rule(Tokenizer& tokenizer);
    void parse_subninja(Tokenizer& tokenizer);

    std::filesystem::path source_filename;
    std::filesystem::path build_filename;

    const File* previous = nullptr;

    std::unordered_set<std::string> outputs;
    std::unordered_map<std::string, Rule> rules;
    std::vector<Build> builds;
    Bindings bindings;
    Text defaults;
    std::vector<std::filesystem::path> subninjas;
    std::vector<std::unique_ptr<File>> subfiles;
};

#endif // FILE_H
