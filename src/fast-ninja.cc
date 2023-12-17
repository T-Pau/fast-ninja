/*
mask.cc -- main file

Copyright (C) Dieter Baron

The authors can be contacted at <mask@tpau.group>

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

#include "config.h"

#include "Command.h"
#include "Tokenizer.h"
#include <iostream>

class fast_ninja: public Command {
public:
    fast_ninja(): Command(options, "top-source-directory", "fast-ninja") {}
    virtual ~fast_ninja() = default;

protected:
    void process() override;
    void create_output() override;
    size_t maximum_arguments() override {return 1;}
    size_t minimum_arguments() override {return 1;}

private:
    static std::vector<Commandline::Option> options;
};

std::vector<Commandline::Option> fast_ninja::options = {
};


int main(int argc, char *argv[]) {
    auto command = fast_ninja();

    return command.run(argc, argv);
}

void fast_ninja::process() {
    auto filename = arguments.arguments[0];

    auto tokenizer = Tokenizer{filename};

    while (auto token = tokenizer.next()) {
        std::cout << token.type_name();
        if (!token.value.empty()) {
            std::cout << " '" << token.value << "'";
        }
        std::cout << std::endl;
    }
}

void fast_ninja::create_output() {}
