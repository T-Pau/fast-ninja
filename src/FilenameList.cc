/*
FilenameList.cc -- 

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

#include "FilenameList.h"


#include "Exception.h"
#include "File.h"


FilenameList::FilenameList(Tokenizer& tokenizer, Type type) {
    auto scoped = (type == Type::SCOPED);
    force_build = (type == Type::BUILD);

    while (true) {
        auto word = FilenameWord{tokenizer, force_build};
        if (!word.empty()) {
            words.emplace_back(word);
        }

        if (scoped) {
            tokenizer.skip_whitespace();
            if (auto token = tokenizer.next()) {
                if (token.type == Tokenizer::TokenType::END_SCOPE) {
                    return;
                }
                else {
                    tokenizer.unget(token);
                }
            }
            else {
                throw Exception("unterminated scope");
            }
        }
        else {
            if (word.empty()) {
                return;
            }
        }
    }
}

void FilenameList::resolve(const ResolveContext& context) {
    for (auto& word : words) {
        word.resolve(context);
        word.collect_filenames(filenames);
    }
    for (auto& filename: filenames) {
        if (force_build) {
            filename.type = Filename::Type::BUILD;
        }
        filename.resolve(context);
    }
}

void FilenameList::serialize(std::ostream& stream) const {
    auto first = true;
    for (auto& filename : filenames) {
        if (first) {
            first = false;
        }
        else {
            stream << " ";
        }
        stream << filename;
    }
}

std::string FilenameList::string() const {
    auto str = std::string();
    auto first = true;

    for (auto& filename: filenames) {
        if (first) {
            first = false;
        }
        else {
            str += " ";
        }
        str += filename.full_name().string();
    }
    return str;
}

std::ostream& operator<<(std::ostream& stream, const FilenameList& filename_list) {
    filename_list.serialize(stream);
    return stream;
}

void FilenameList::collect_output_files(std::unordered_set<std::filesystem::path>& output_files) const {
    for (auto& filename: filenames) {
        if (filename.type == Filename::Type::BUILD) {
            output_files.insert(filename.full_name());
        }
    }
}
