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
#include <vector>

#include "Bindings.h"
#include "Rule.h"

class Build {
  public:
    Build() = default;
    explicit Build(Tokenizer& tokenizer);
    Build(std::string rule_name, Text outputs, Text inputs, Bindings bindings): rule_name{std::move(rule_name)}, outputs{std::move(outputs)}, inputs{std::move(inputs)}, bindings{std::move(bindings)} {}

    void process(const File& file);
    void process_outputs(const File& file);
    void print(std::ostream& stream) const;

    [[nodiscard]] std::unordered_set<std::string> get_outputs() const;

  private:
    const Rule* rule{};
    std::string rule_name;
    Text outputs;
    Text inputs;
    Bindings bindings;
};

#endif // BUILD_H
