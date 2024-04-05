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
#include "fontprovider.h"

#include "log.h"

using namespace muse::draw;

int FontProvider::addSymbolFont(const mu::String& family, const mu::io::path_t& path)
{
    //! NOTE Adding fonts in the fontsmodule
    UNUSED(family);
    UNUSED(path);
    return 1;
}

int FontProvider::addTextFont(const mu::io::path_t& path)
{
    //! NOTE Adding fonts in the fontsmodule
    UNUSED(path);
    return 1;
}

void FontProvider::insertSubstitution(const mu::String& from, const mu::String& to)
{
    //! NOTE Instead of substitution, default fonts are used
    //! Set in the fontsmodule
    UNUSED(from);
    UNUSED(to);
}

double FontProvider::lineSpacing(const muse::draw::Font& f) const
{
    return fontsEngine()->lineSpacing(f);
}

double FontProvider::xHeight(const muse::draw::Font& f) const
{
    return fontsEngine()->xHeight(f);
}

double FontProvider::height(const muse::draw::Font& f) const
{
    return fontsEngine()->height(f);
}

double FontProvider::ascent(const muse::draw::Font& f) const
{
    return fontsEngine()->ascent(f);
}

double FontProvider::descent(const muse::draw::Font& f) const
{
    return fontsEngine()->descent(f);
}

bool FontProvider::inFont(const muse::draw::Font& f, mu::Char ch) const
{
    return inFontUcs4(f, static_cast<char32_t>(ch.unicode()));
}

bool FontProvider::inFontUcs4(const muse::draw::Font& f, char32_t ucs4) const
{
    return fontsEngine()->inFontUcs4(f, ucs4);
}

// Text
double FontProvider::horizontalAdvance(const muse::draw::Font& f, const mu::String& string) const
{
    return fontsEngine()->horizontalAdvance(f, string.toStdU32String());
}

double FontProvider::horizontalAdvance(const muse::draw::Font& f, const mu::Char& ch) const
{
    return fontsEngine()->horizontalAdvance(f, ch.unicode());
}

mu::RectF FontProvider::boundingRect(const muse::draw::Font& f, const mu::String& string) const
{
    return fontsEngine()->boundingRect(f, string.toStdU32String());
}

mu::RectF FontProvider::boundingRect(const muse::draw::Font& f, const mu::Char& ch) const
{
    return fontsEngine()->boundingRect(f, ch.unicode());
}

mu::RectF FontProvider::boundingRect(const muse::draw::Font& f, const mu::RectF& r, int flags, const mu::String& string) const
{
    UNUSED(r);
    UNUSED(flags);
    return boundingRect(f, string);
}

mu::RectF FontProvider::tightBoundingRect(const muse::draw::Font& f, const mu::String& string) const
{
    return fontsEngine()->tightBoundingRect(f, string.toStdU32String());
}

// Score symbols
mu::RectF FontProvider::symBBox(const muse::draw::Font& f, char32_t ucs4, double DPI_F) const
{
    UNUSED(DPI_F);
    return fontsEngine()->symBBox(f, ucs4);
}

double FontProvider::symAdvance(const muse::draw::Font& f, char32_t ucs4, double DPI_F) const
{
    UNUSED(DPI_F);
    return fontsEngine()->symAdvance(f, ucs4);
}
