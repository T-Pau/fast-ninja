/*
Dependencies.cc --

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

#include "Dependencies.h"


#include <Exception.h>

Dependencies::Dependencies(Tokenizer& tokenizer, bool is_build) {
    auto type = is_build ? FilenameList::BUILD : FilenameList::INLINE;

    direct = FilenameList{tokenizer, type};

    while (true) {
        tokenizer.skip_space();
        const auto token = tokenizer.next();
        switch (token.type) {
            case Tokenizer::TokenType::END:
            case Tokenizer::TokenType::NEWLINE:
            case Tokenizer::TokenType::COLON:
                tokenizer.unget(token);
                return;

            case Tokenizer::TokenType::IMPLICIT_DEPENDENCY:
                implicit = FilenameList(tokenizer, type);
                break;

            case Tokenizer::TokenType::ORDER_DEPENDENCY:
                order = FilenameList(tokenizer, type);
                break;

            case Tokenizer::TokenType::VALIDATION_DEPENDENCY:
                validation = FilenameList(tokenizer, type);
                break;

            default:
                throw Exception("internal error: %s not included in filename", token.type_name().c_str());
        }
    }
}


void Dependencies::resolve(const Scope& scope) {
    auto context = ResolveContext{scope};
    direct.resolve(context);
    implicit.resolve(context);
    order.resolve(context);
    validation.resolve(context);
}

void Dependencies::serialize(std::ostream& stream) const {
    auto first = true;
    if (!direct.empty()) {
        stream << direct;
        first = false;
    }
    if (!implicit.empty()) {
        if (first) {
            first = false;
        }
        else {
            stream << " ";
        }
        stream << "| " << implicit;
    }
    if (!order.empty()) {
        if (first) {
            first = false;
        }
        else {
            stream << " ";
        }
        stream << "|| " << order;
    }
    if (!validation.empty()) {
        if (first) {
            first = false;
        }
        else {
            stream << " ";
        }
        stream << "|@ " << validation;
    }
}

void Dependencies::collect_output_files(std::unordered_set<std::string>& output_files) const {
    direct.collect_output_files(output_files);
    implicit.collect_output_files(output_files);
    order.collect_output_files(output_files);
    validation.collect_output_files(output_files);
}

std::ostream& operator<<(std::ostream& stream, const Dependencies& dependencies) {
    dependencies.serialize(stream);
    return stream;
}
