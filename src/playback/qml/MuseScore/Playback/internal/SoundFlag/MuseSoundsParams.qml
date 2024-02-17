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
import QtQuick 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Item {
    id: root

    property var model: null

    property NavigationSection navigationPanelSection: null
    property int navigationPanelOrderStart: 0
    property int navigationPanelOrderEnd: playingTechniquesGridView.navigationPanel.order

    height: !prv.noOptions ? content.childrenRect.height : noOptionsLabel.implicitHeight

    QtObject {
        id: prv

        property bool noOptions: !modifySoundView.hasPresets && !playingTechniquesGridView.hasPlayingTechniques
    }

    Column {
        id: content

        width: parent.width

        spacing: 12

        ParamsGridView {
            id: modifySoundView

            property bool hasPresets: Boolean(model) && model.length !== 0

            width: parent.width

            title: qsTrc("playback", "Modify sound")
            model: root.model.availablePresets
            selectionModel: root.model.presetCodes

            navigationPanel.section: root.navigationPanelSection
            navigationPanel.order: root.navigationPanelOrderStart

            visible: hasPresets

            onToggleParamRequested: {
                root.model.togglePreset(paramCode)
            }
        }

        ParamsGridView {
            id: playingTechniquesGridView

            property bool hasPlayingTechniques: Boolean(model) && model.length !== 0

            width: parent.width

            title: qsTrc("playback", "Choose playing technique")
            model: root.model.availablePlayingTechniques
            selectionModel: root.model.playingTechniquesCodes

            navigationPanel.section: root.navigationPanelSection
            navigationPanel.order: root.navigationPanelOrderStart + 1

            visible: hasPlayingTechniques

            onToggleParamRequested: {
                root.model.togglePlayingTechnique(paramCode)
            }
        }
    }

    StyledTextLabel {
        id: noOptionsLabel

        text: qsTrc("playback", "Sound flag options are not available for this sound.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.Wrap

        visible: prv.noOptions
    }
}
