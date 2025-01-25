/*
FileName.cc --

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

#include "Filename.h"


#include "Exception.h"
#include "FastNinjaUtil.h"
#include "File.h"
#include <DiagnosticOutput.h>

void Filename::resolve(const ResolveContext& context) {
    if (!context.classify_filenames) {
        return;
    }
    const auto file = context.scope.get_file();

    if (type == Type::UNKNOWN) {
        if (context.scope.is_output_file((file->build_directory / name).lexically_normal())) {
            type = Type::BUILD;
        }
        else if (std::filesystem::exists(file->source_directory / name)) {
            type = Type::SOURCE;
        }
    }

    switch (type) {
        case Type::BUILD:
            if (prefix.empty()) {
                prefix = file->build_directory;
            }
            break;

        case Type::COMPLETE:
            prefix = "";
            break;

        case Type::SOURCE:
            if (prefix.empty()) {
                prefix = file->source_directory;
            }
            if (!std::filesystem::exists(full_name())) {
                DiagnosticOutput::global.error({}, location) << "source file '" << full_name().string() << "' does not exist";
                throw Exception();
            }
            break;

        case Type::UNKNOWN:
            if (prefix.empty()) {
                prefix = file->source_directory;
            }
            DiagnosticOutput::global.error({}, location) << "unknown file '" << full_name().string() << "'"; // TODO: include sub-directory
            throw Exception();
    }
}

std::filesystem::path Filename::full_name() const {
    if (prefix.empty()) {
        return std::filesystem::path(name).lexically_normal();
    }
    else {
        return (prefix / name).lexically_normal();
    }
}

bool Filename::operator<(const Filename& other) const {
    if (type != other.type) {
        return type < other.type;
    }
    return name < other.name;
}

std::ostream& operator<<(std::ostream& stream, const Filename& file_name) {
    stream << dollar_escape(file_name.full_name().generic_string());
    return stream;
}
