/*
Build.cc --

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

#include "Build.h"


#include "File.h"
#include <Exception.h>

Build::Build(const File* file, Tokenizer& tokenizer): ScopedDirective(file) {
    outputs = Dependencies{tokenizer, true};
    tokenizer.expect(Tokenizer::TokenType::COLON, Tokenizer::Skip::SPACE);
    rule_name = tokenizer.expect(Tokenizer::TokenType::WORD, Tokenizer::Skip::SPACE).string();
    inputs = Dependencies{tokenizer, false};
    bindings = Bindings{tokenizer};
}

Build::Build(const File* file, std::string rule_name, Dependencies outputs, Dependencies inputs, Bindings bindings): ScopedDirective{file, std::move(bindings)}, rule_name{std::move(rule_name)}, outputs{std::move(outputs)}, inputs{std::move(inputs)} {}

void Build::process(const File& file) {
    rule = file.find_rule(rule_name);
    if (!rule) {
        throw Exception("unknown rule %s", rule_name.c_str());
    }
    inputs.resolve(file);
    bindings.resolve(file);
}

void Build::process_outputs(const File& file) {
    outputs.resolve(file);
}

void Build::print(std::ostream& stream) const {
    stream << std::endl << "build " << outputs << " : " << rule_name << " " << inputs << std::endl;
    bindings.print(stream, "    ");
}
