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
#include "extpluginsloader.h"

#include "global/io/dir.h"
#include "global/io/file.h"
#include "global/io/fileinfo.h"

#include "log.h"

using namespace mu::extensions;
using namespace mu::extensions::legacy;

ManifestList ExtPluginsLoader::loadManifesList(const io::path_t& defPath, const io::path_t& extPath) const
{
    TRACEFUNC;

    LOGD() << "try load extensions, def: " << defPath << ", user: " << extPath;

    ManifestList defaultManifests = manifesList(defPath);
    ManifestList externalManifests = manifesList(extPath);

    ManifestList retList;
    for (const Manifest& m : defaultManifests) {
        if (!m.enabled || !m.isValid()) {
            continue;
        }
        retList.push_back(m);
    }

    for (const Manifest& m : externalManifests) {
        if (!m.enabled || !m.isValid()) {
            continue;
        }
        retList.push_back(m);
    }

    return retList;
}

ManifestList ExtPluginsLoader::manifesList(const io::path_t& rootPath) const
{
    ManifestList manifests;
    io::paths_t paths = qmlsPaths(rootPath);
    for (const io::path_t& path : paths) {
        Manifest manifest = parseManifest(path);
        resolvePaths(manifest, io::FileInfo(path).dirPath());
        manifests.push_back(manifest);
    }

    return manifests;
}

mu::io::paths_t ExtPluginsLoader::qmlsPaths(const io::path_t& rootPath) const
{
    RetVal<io::paths_t> paths = io::Dir::scanFiles(rootPath, { "*.qml" });
    if (!paths.ret) {
        LOGE() << "failed scan files, err: " << paths.ret.toString();
    }
    return paths.val;
}

Manifest ExtPluginsLoader::parseManifest(const io::path_t& path) const
{
    ByteArray data;
    Ret ret = io::File::readFile(path, data);
    if (!ret) {
        LOGE() << "failed read file: " << path << ", err: " << ret.toString();
        return Manifest();
    }

    io::FileInfo fi(path);

    Manifest m;
    m.uri = Uri("musescore://extensions/legacy/" + fi.baseName().toLower().toStdString());
    m.type = Type::Macros;
    m.apiversion = 1;
    m.qmlFilePath = fi.fileName();
    m.enabled = true;
    m.visible = true;

    auto dropQuotes = [](const String& str) {
        if (str.size() < 3) {
            return String();
        }
        return str.mid(1, str.size() - 2);
    };

    int needProperties = 3; // title, description, pluginType
    int propertiesFound = 0;
    String content = String::fromUtf8(data);
    size_t current, previous = 0;
    current = content.indexOf(u"\n");
    while (current != std::string::npos) {
        String line = content.mid(previous, current - previous).trimmed();

        if (line.startsWith(u'/')) { // comment
            // noop
        } else if (line.startsWith(u"title:")) {
            m.title = dropQuotes(line.mid(6).trimmed());
            ++propertiesFound;
        } else if (line.startsWith(u"description:")) {
            m.description = dropQuotes(line.mid(12).trimmed());
            ++propertiesFound;
        } else if (line.startsWith(u"pluginType:")) {
            String pluginType = dropQuotes(line.mid(11).trimmed());
            if (pluginType == "dialog") {
                m.type = Type::Form;
            }
            ++propertiesFound;
        }

        if (propertiesFound == needProperties) {
            break;
        }

        previous = current + 1;
        current = content.indexOf(u"\n", previous);
    }

    return m;
}

void ExtPluginsLoader::resolvePaths(Manifest& m, const io::path_t& rootDirPath) const
{
    m.qmlFilePath = rootDirPath + "/" + m.qmlFilePath;
}
