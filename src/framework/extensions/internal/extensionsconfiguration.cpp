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
#include "extensionsconfiguration.h"

#include "global/settings.h"
#include "global/serialization/json.h"
#include "global/io/file.h"
#include "multiinstances/resourcelockguard.h"

#include "log.h"

using namespace mu::extensions;

static const mu::Settings::Key USER_PLUGINS_PATH("plugins", "application/paths/myPlugins");

static const std::string EXTENSIONS_RESOURCE_NAME("EXTENSIONS");

mu::io::path_t ExtensionsConfiguration::defaultPath() const
{
    return globalConfiguration()->appDataPath() + "/extensions";
}

mu::io::path_t ExtensionsConfiguration::userPath() const
{
    return globalConfiguration()->userAppDataPath() + "/extensions";
}

mu::io::path_t ExtensionsConfiguration::pluginsDefaultPath() const
{
    return globalConfiguration()->appDataPath() + "/plugins";
}

mu::io::path_t ExtensionsConfiguration::pluginsUserPath() const
{
    return settings()->value(USER_PLUGINS_PATH).toPath();
}

mu::Ret ExtensionsConfiguration::setManifestConfigs(const std::map<Uri, Manifest::Config>& configs)
{
    JsonArray arr;
    for (const auto& p : configs) {
        JsonObject obj;
        obj["uri"] = p.first.toString();
        obj["enabled"] = p.second.enabled;
        obj["shortcuts"] = p.second.shortcuts;

        arr.append(obj);
    }

    Ret ret;
    ByteArray data = JsonDocument(arr).toJson();
    {
        mi::WriteResourceLockGuard lock_guard(multiInstancesProvider.get(), EXTENSIONS_RESOURCE_NAME);
        ret = io::File::writeFile(pluginsUserPath() + "/config.json", data);
    }

    if (!ret) {
        LOGE() << "failed write config data, err: " << ret.toString();
    }

    return ret;
}

std::map<mu::Uri, Manifest::Config> ExtensionsConfiguration::manifestConfigs() const
{
    ByteArray data;
    {
        TRACEFUNC;

        mi::ReadResourceLockGuard lock_guard(multiInstancesProvider.get(), EXTENSIONS_RESOURCE_NAME);
        Ret ret = io::File::readFile(pluginsUserPath() + "/config.json", data);
        if (!ret) {
            LOGE() << "failed read config data, err: " << ret.toString();
            return {};
        }
    }

    std::string err;
    JsonDocument doc = JsonDocument::fromJson(data, &err);
    if (!err.empty()) {
        LOGE() << "failed parse json, err: " << err;
        return {};
    }

    if (!doc.isArray()) {
        LOGE() << "bad format of config file";
        return {};
    }

    JsonArray arr = doc.rootArray();

    std::map<mu::Uri, Manifest::Config> result;
    for (size_t i = 0; i < arr.size(); ++i) {
        JsonObject obj = arr.at(i).toObject();

        Uri uri;
        Manifest::Config c;
        if (obj.contains("uri")) {
            uri = Uri(obj.value("uri").toStdString());
        } else {
            uri = Uri("musescore://extensions/v1/" + obj.value("codeKey").toStdString());
        }

        c.enabled = obj.value("enabled").toBool();
        c.shortcuts = obj.value("shortcuts").toStdString();

        if (uri.isValid()) {
            result[uri] = c;
        }
    }

    return result;
}
