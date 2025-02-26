/*
Build.h --

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

#ifndef BUILD_H
#define BUILD_H

#include <unordered_set>

#include "Bindings.h"
#include "Dependencies.h"
#include "Rule.h"
#include "ScopedDirective.h"

class Build: public ScopedDirective {
  public:
    explicit Build(const File* file, Tokenizer& tokenizer);
    Build(const File* file, std::string rule_name, Dependencies outputs, Dependencies inputs, Bindings bindings);

    [[nodiscard]] bool is_phony() const {return rule_name == "phony";}
    void process(const File& file);
    void process_outputs(const File& file);
    void print(std::ostream& stream) const;

    void collect_output_files(std::unordered_set<std::string>& output_files) const;

  private:
    const Rule* rule{};
    std::string rule_name;
    Dependencies outputs;
    Dependencies inputs;
};

#endif // BUILD_H
