/*
Text.cc --

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

#include "Text.h"

#include "Exception.h"
#include "File.h"

Text::Text(Tokenizer& tokenizer) {
    tokenizer.skip_space();
    while (true) {
        words.emplace_back(tokenizer);
        auto token = tokenizer.next();
        if (token.type == Tokenizer::TokenType::NEWLINE) {
            break;
        }
        words.emplace_back(token.string());
    }
}

std::ostream& operator<<(std::ostream& stream, const Text& text) {
    text.print(stream);
    return stream;
}

void Text::print(std::ostream& stream) const {
    for (auto& word: words) {
        stream << word;
    }
}

void Text::resolve(const ResolveContext& context) {
    for (auto& word : words) {
        word.resolve(context);
    }
}

std::string Text::string() const {
    auto result = std::string{};

    for (auto& word : words) {
        result += word.string();
    }

    return result;
}
