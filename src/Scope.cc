/*
Scope.cc --

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

#include "Scope.h"

#include "File.h"

Variable* Scope::get_variable(const std::string& name) const {
    auto scope = this;
    while (scope) {
        auto it = scope->bindings.find(name);
        if (it != scope->bindings.end()) {
            return it->second.get();
        }
        scope = scope->next;
    }

    return {};
}

const File* Scope::get_file() const {
    auto scope = this;
    while (scope) {
        if (const auto file = dynamic_cast<const File*>(scope)) {
            return file;
        }
        scope = scope->next;
    }

    return {};
}

const Scope* Scope::top() const {
    auto scope = this;

    while (!scope->is_top()) {
        scope = scope->next;
    }

    return scope;
}

const File* Scope::as_file() const {
    return dynamic_cast<const File*>(this);
}

bool Scope::is_output_file(const std::filesystem::path& file) const {
    if (auto top_file = top()->as_file()) {
        return top_file->is_output(file);
    }
    else {
        // TODO: internal error?
        return false;
    }
}
