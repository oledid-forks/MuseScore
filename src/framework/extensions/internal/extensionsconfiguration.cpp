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

using namespace mu::extensions;

static const mu::Settings::Key USER_PLUGINS_PATH("plugins", "application/paths/myPlugins");

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
