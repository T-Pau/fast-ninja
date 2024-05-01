/*
VariableDependencies.cc --

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

#include "VariableDependencies.h"
#include "Exception.h"

#include <iostream>

VariableDependencies::VariableDependencies(const std::unordered_map<std::string, std::shared_ptr<Variable>>& variables) {
    for (auto& pair: variables) {
        unresolved[pair.first] = {};
        known_variables.insert(pair.first);
    }
}
void VariableDependencies::update(const std::string& name, const std::unordered_set<std::string>& dependencies) {
    if (dependencies.empty()) {
        resolved.insert(name);
        unresolved.erase(name);
    }
    else {
        if (!std::all_of(dependencies.begin(), dependencies.end(), [this](const auto& name){
                                                                       return known_variables.contains(name);
                                                                   })) {
            throw Exception("unknwon variable"); // TODO: include name
        }
        unresolved[name] = dependencies;
    }
}

std::unordered_set<std::string> VariableDependencies::get_next() const {
    if (finished()) {
        return {};
    }

    std::unordered_set<std::string> next;

    for (auto& pair: unresolved) {
        if (std::all_of(pair.second.begin(), pair.second.end(), [this](const std::string& dependency) {
               return resolved.contains(dependency);
            })) {
            next.insert(pair.first);
        }
    }

    if (next.empty()) {
        throw Exception("cycle in variable definition"); // TODO: include list of variables
    }

    return next;
}
