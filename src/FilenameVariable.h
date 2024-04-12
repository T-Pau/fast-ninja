/*
FilenameVariable.h --

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

#ifndef FILENAMEVARIABLE_H
#define FILENAMEVARIABLE_H

#include "FilenameList.h"
#include "ResolveContext.h"
#include "Variable.h"

#include <utility>

class FilenameVariable : public Variable {
  public:
    FilenameVariable(std::string name, Tokenizer& tokenizer);
    FilenameVariable(std::string name, FilenameList value): Variable(std::move(name)), value{std::move(value)} {}

    void resolve_sub(const ResolveContext& context) override {value.resolve(context);}
    void print_definition(std::ostream& stream) const override {} // TODO
    void print_use(std::ostream& stream) const override {} // TODO
    [[nodiscard]] std::string string() const override {return value.string();}
    void collect_filenames(std::vector<Filename>& collector) const {return value.collect_filenames(collector);}

private:
    FilenameList value;
};

#endif // FILENAMEVARIABLE_H
