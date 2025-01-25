#ifndef EXPLICITFILENAME_H
#define EXPLICITFILENAME_H

/*
FilenameWord.h --

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

#include "Filename.h"

#include <variant>
#include <vector>

#include "VariableReference.h"


class FilenameList;

class FilenameWord {
  public:
    explicit FilenameWord(Tokenizer& tokenizer, bool force_build = false);
    explicit FilenameWord(std::string word): elements{std::move(word)} {}

    [[nodiscard]] bool empty() const {return elements.empty();}
    [[nodiscard]] bool is_resolved() const {return resolved;}
    void resolve(const ResolveContext& context);

    void collect_filenames(std::vector<Filename>& filenames) const;

    Location location;

private:
    std::vector<std::variant<std::string, VariableReference, const Variable*>> elements;
    std::optional<Filename> filename;
    bool force_build{false};
    bool resolved{true};
};

#endif // EXPLICITFILENAME_H
