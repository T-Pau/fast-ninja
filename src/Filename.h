#ifndef FILENAME_H
#define FILENAME_H

/*
Filename.h --

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

#include <filesystem>

#include "ResolveContext.h"

class Scope;

class Filename {
public:
    enum class Type {
        BUILD,
        COMPLETE,
        SOURCE,
        UNKNOWN
    };

    explicit Filename(Location location, std::string name): location{std::move(location)}, name{std::move(name)} {}
    Filename(Location location, Type type, std::string name): location{std::move(location)}, type{type}, name{std::move(name)} {}
    Filename() = default;

    void resolve(const ResolveContext& context);

    [[nodiscard]] std::filesystem::path full_name() const;

    // TODO: consider prefix?
    bool operator<(const Filename& other) const;
    bool operator==(const Filename& other) const {return type == other.type && name == other.name;}

    Type type{Type::UNKNOWN};
    std::string name;
    std::filesystem::path prefix;
    Location location;
};

std::ostream& operator<<(std::ostream& stream, const Filename& file_name);

#endif //FILENAME_H
