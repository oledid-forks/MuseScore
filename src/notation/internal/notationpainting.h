/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTATIONPAINTING_H
#define MU_NOTATION_NOTATIONPAINTING_H

#include "../inotationpainting.h"

#include "modularity/ioc.h"
#include "../inotationconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "engraving/rendering/iscorerenderer.h"
#include "ui/iuiconfiguration.h"

namespace mu::engraving {
class Score;
class Page;
}

namespace mu::notation {
class Notation;
class NotationPainting : public INotationPainting
{
    INJECT(INotationConfiguration, configuration)
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration)
    INJECT(engraving::rendering::IScoreRenderer, scoreRenderer)
    INJECT(ui::IUiConfiguration, uiConfiguration)

public:
    NotationPainting(Notation* notation);

    void setViewMode(const ViewMode& viewMode) override;
    ViewMode viewMode() const override;
    async::Notification viewModeChanged() const override;

    int pageCount() const override;
    SizeF pageSizeInch() const override;
    SizeF pageSizeInch(const Options& opt) const override;

    void paintView(muse::draw::Painter* painter, const RectF& frameRect, bool isPrinting) override;
    void paintPdf(muse::draw::Painter* painter, const Options& opt) override;
    void paintPrint(muse::draw::Painter* painter, const Options& opt) override;
    void paintPng(muse::draw::Painter* painter, const Options& opt) override;

private:
    mu::engraving::Score* score() const;

    bool isPaintPageBorder() const;
    void doPaint(muse::draw::Painter* painter, const Options& opt);
    void paintPageBorder(muse::draw::Painter* painter, const mu::engraving::Page* page) const;
    void paintPageSheet(muse::draw::Painter* painter, const engraving::Page* page, const RectF& pageRect, bool printPageBackground) const;

    Notation* m_notation = nullptr;

    async::Notification m_viewModeChanged;
};
}

#endif // MU_NOTATION_NOTATIONPAINTING_H
