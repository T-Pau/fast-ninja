/*
TextVariable.h -- 

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

#ifndef TEXTVARIABLE_H
#define TEXTVARIABLE_H

#include "Variable.h"
#include "Text.h"

class TextVariable: public Variable {
public:
    TextVariable(std::string name, Tokenizer& tokenizer): Variable(std::move(name)), value(tokenizer) {}
    TextVariable(std::string name, Text value): Variable(std::move(name)), value{std::move(value)} {}

    void resolve(const ResolveContext& scope) override;
    void print_definition(std::ostream& stream) const override;
    [[nodiscard]] std::string string() const override {return value.string();}
    [[nodiscard]] bool contains_unknown_file() const override {return value.contains_unknown_file();}
    bool is_resolved() const override {return value.is_resolved();}

private:
    Text value;
};

#endif //TEXTVARIABLE_H
