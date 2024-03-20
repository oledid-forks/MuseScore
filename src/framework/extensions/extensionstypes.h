/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_EXTENSIONS_EXTENSIONSTYPES_H
#define MU_EXTENSIONS_EXTENSIONSTYPES_H

#include <vector>
#include <map>

#include "global/types/uri.h"
#include "global/types/string.h"
#include "global/io/path.h"
#include "global/types/translatablestring.h"

namespace mu::extensions {
//! NOTE Api versions:
//! 1 - plugins from 3х
//! 2 - extensions
constexpr int DEFAULT_API_VERSION = 2;

enum class Type {
    Undefined = 0,
    Form,       // Have UI, controls, user interaction
    Macros      // Without UI, they just do some script
};

static inline Type typeFromString(const std::string& str)
{
    if (str == "form") {
        return Type::Form;
    } else if (str == "macros") {
        return Type::Macros;
    }
    return Type::Undefined;
}

static inline std::string typeToString(const Type& type)
{
    switch (type) {
    case Type::Undefined: return "undefined";
    case Type::Form: return "form";
    case Type::Macros: return "macros";
    }
    return std::string();
}

enum Filter {
    Enabled,
    All
};

/*
manifest.json
{

"uri": String,                    // Example: musescore://module/target/name
"type": String,                   // Values: form, macros
"title": String,                  //
"description": String,            //
"apiversion": String              // Optional default 2

"qmlFilePath": String             // Path (name) of qml file of form
"jsFilePath": String              // Path (name) of js file of macros
}*/

struct Manifest {
    Uri uri;
    Type type = Type::Undefined;
    String title;
    String description;
    String category;
    mu::io::path_t thumbnail;
    String version;
    int apiversion = DEFAULT_API_VERSION;
    bool requiresScore = true;

    mu::io::path_t qmlFilePath;
    mu::io::path_t jsFilePath;

    struct Config {
        bool enabled = false;
        std::string shortcuts;
    } config;

    bool isValid() const { return type != Type::Undefined && uri.isValid(); }
};

using ManifestList = std::vector<Manifest>;

using KnownCategories = std::map<std::string /*name*/, TranslatableString /*title*/>;
}

#endif // MU_EXTENSIONS_EXTENSIONSTYPES_H
