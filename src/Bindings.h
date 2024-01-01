/*
Bindings.h --

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

#ifndef BINDINGS_H
#define BINDINGS_H

#include <map>

#include "Variable.h"

class Bindings {
  public:
    Bindings() = default;
    explicit Bindings(Tokenizer& tokenizer);
    explicit Bindings(const std::vector<Variable>& variable_list);

    void print(std::ostream& stream, const std::string& indent) const;
    void process(const File& file);
    void add(const std::string& name, Variable variable) {variables[name] = std::move(variable);}

    [[nodiscard]] auto empty() const {return variables.empty();}
    auto begin() { return variables.begin(); }

    auto end() { return variables.end(); }

    [[nodiscard]] auto begin() const { return variables.begin(); }

    [[nodiscard]] auto end() const { return variables.end(); }
    auto find(const std::string& name) {return variables.find(name);}
    auto find(const std::string& name) const {return variables.find(name);}

  private:
    std::map<std::string, Variable> variables;
};

#endif // BINDINGS_H
