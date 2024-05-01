#ifndef FILENAMELIST_H
#define FILENAMELIST_H

/*
FilenameList.h --

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

#include <utility>

#include "Filename.h"
#include "FilenameWord.h"

class FilenameList {
  public:
    enum Type {
        BUILD,
        INLINE,
        SCOPED
    };
    explicit FilenameList(Tokenizer& tokenizer, Type type);
    explicit FilenameList(Filename filename): filenames({std::move(filename)}) {}
    explicit FilenameList(bool force_build = false): force_build{force_build} {}
    explicit FilenameList(std::vector<Filename> filenames): filenames(std::move(filenames)) {}

    void resolve(const ResolveContext& context);
    [[nodiscard]] bool empty() const {return words.empty() && filenames.empty();}
    [[nodiscard]] bool is_resolved() const {return resolved;}
    void serialize(std::ostream& stream) const;
    [[nodiscard]] std::string string() const;
    [[nodiscard]] bool contains_unknown_file() const {return false;} // TODO
    void collect_output_files(std::unordered_set<std::string>& output_files) const;

    void collect_filenames(std::vector<Filename>& collector) const {collector.insert(collector.end(), filenames.begin(), filenames.end());}

  private:
    std::vector<FilenameWord> words;
    std::vector<Filename> filenames;
    bool force_build{false};
    bool resolved{true};
};

std::ostream& operator<<(std::ostream& stream, const FilenameList& filename_list);

#endif // FILENAMELIST_H
