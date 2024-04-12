#ifndef DEPENDENCIES_H
#define DEPENDENCIES_H

/*
Dependencies.h --

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

#include <vector>

#include "Filename.h"
#include "FilenameList.h"
#include "Tokenizer.h"

class Dependencies {
  public:
    Dependencies(Tokenizer& tokenizer, bool force_build);
    Dependencies(FilenameList direct): direct(std::move(direct)) {}
    Dependencies() = default;

    void resolve(const Scope& scope);
    void collect_output_files(std::unordered_set<std::filesystem::path>& output_files) const;
    void mark_as_build();
    void serialize(std::ostream& stream) const;

  private:
    FilenameList direct;
    FilenameList implicit;
    FilenameList order;
    FilenameList validation;
};

std::ostream& operator<<(std::ostream& stream, const Dependencies& dependencies);
#endif // DEPENDENCIES_H
