/*
Variable.h -- 

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

#ifndef VARIABLE_H
#define VARIABLE_H

#include <string>

#include "Tokenizer.h"

class ResolveContext;
class FilenameVariable;
class TextVariable;

class Variable {
public:
    explicit Variable(std::string name): name{std::move(name)} {}
    Variable() = default;
    virtual ~Variable() = default;

    [[nodiscard]] const FilenameVariable* as_filename() const;
    [[nodiscard]] const TextVariable* as_text() const;
    [[nodiscard]] bool is_filename() const {return as_filename();}
    [[nodiscard]] bool is_text() const {return as_text();}

    void resolve(const ResolveContext& context);
    virtual void resolve_sub(const ResolveContext& context) = 0;
    virtual void print_definition(std::ostream& stream) const = 0;
    [[nodiscard]] virtual std::string string() const = 0;

    std::string name;

private:
    bool resolved{false};
};



#endif //VARIABLE_H
