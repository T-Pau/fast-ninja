#ifndef SCOPE_H
#define SCOPE_H

/*
Scope.h --

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

#include "Bindings.h"

class File;

class Scope {
public:
    Scope() = default;
    explicit Scope(const Scope* next): next{next} {}
    Scope(const Scope* next, Bindings bindings): next{next}, bindings{std::move(bindings)} {}
    virtual ~Scope() = default;

    [[nodiscard]] bool is_top() const {return !next;}
    [[nodiscard]] const Scope* top() const;
    [[nodiscard]] const File* as_file() const;
    [[nodiscard]] bool is_file() const {return as_file();}

    [[nodiscard]] Variable* get_variable(const std::string& name) const;
    [[nodiscard]] const File* get_file() const;
    [[nodiscard]] bool is_output_file(const std::filesystem::path& file) const;

    virtual void polymorphic() const {}

protected:
    const Scope* next{};
    Bindings bindings{};
};

#endif //SCOPE_H
