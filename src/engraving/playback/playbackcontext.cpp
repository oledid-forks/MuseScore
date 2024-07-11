/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "playbackcontext.h"

#include "dom/dynamic.h"
#include "dom/hairpin.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/playtechannotation.h"
#include "dom/stafftext.h"
#include "dom/soundflag.h"
#include "dom/repeatlist.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/measurerepeat.h"

#include "utils/arrangementutils.h"
#include "utils/expressionutils.h"
#include "types/typesconv.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static bool soundFlagPlayable(const SoundFlag* flag)
{
    if (flag && flag->play()) {
        return !flag->soundPresets().empty() || !flag->playingTechnique().empty();
    }

    return false;
}

dynamic_level_t PlaybackContext::appliableDynamicLevel(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    auto dynamicsIt = m_dynamicsByTrack.find(trackIdx);
    if (dynamicsIt == m_dynamicsByTrack.end()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    const DynamicMap& dynamics = dynamicsIt->second;
    auto it = muse::findLessOrEqual(dynamics, nominalPositionTick);
    if (it == dynamics.end()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    return it->second.level;
}

ArticulationType PlaybackContext::persistentArticulationType(const int nominalPositionTick) const
{
    auto it = findLessOrEqual(m_playTechniquesMap, nominalPositionTick);
    if (it == m_playTechniquesMap.cend()) {
        return mpe::ArticulationType::Standard;
    }

    return it->second;
}

PlaybackParamList PlaybackContext::playbackParams(const track_idx_t trackIdx, const int nominalPositionTick) const
{
    auto paramsIt = m_playbackParamByTrack.find(trackIdx);
    if (paramsIt == m_playbackParamByTrack.end()) {
        return {};
    }

    const ParamMap& params = paramsIt->second;
    auto it = muse::findLessOrEqual(params, nominalPositionTick);
    if (it == params.end()) {
        return {};
    }

    return it->second;
}

PlaybackParamLayers PlaybackContext::playbackParamLayers(const Score* score) const
{
    PlaybackParamLayers result;

    for (const auto& params : m_playbackParamByTrack) {
        PlaybackParamMap paramMap;
        for (const auto& pair : params.second) {
            paramMap.emplace(timestampFromTicks(score, pair.first), pair.second);
        }

        result.emplace(static_cast<layer_idx_t>(params.first), std::move(paramMap));
    }

    return result;
}

DynamicLevelLayers PlaybackContext::dynamicLevelLayers(const Score* score) const
{
    DynamicLevelLayers result;

    for (const auto& dynamics : m_dynamicsByTrack) {
        DynamicLevelMap dynamicLevelMap;
        for (const auto& dynamic : dynamics.second) {
            dynamicLevelMap.emplace(timestampFromTicks(score, dynamic.first), dynamic.second.level);
        }

        result.emplace(static_cast<layer_idx_t>(dynamics.first), std::move(dynamicLevelMap));
    }

    return result;
}

void PlaybackContext::update(const ID partId, const Score* score)
{
    const Part* part = score->partById(partId);
    IF_ASSERT_FAILED(part) {
        return;
    }

    // cache them for optimization
    m_partStartTrack = part->startTrack();
    m_partEndTrack = part->endTrack();

    IF_ASSERT_FAILED(m_partStartTrack <= m_partEndTrack) {
        return;
    }

    const size_t ntracks = m_partEndTrack - m_partStartTrack;
    m_dynamicsByTrack.reserve(ntracks);
    m_playbackParamByTrack.reserve(ntracks);

    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        std::vector<const MeasureRepeat*> measureRepeats;
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                int segmentStartTick = segment->tick().ticks() + tickPositionOffset;

                for (track_idx_t track = m_partStartTrack; track < m_partEndTrack; ++track) {
                    const EngravingItem* item = segment->elementAt(track);
                    if (!item || !item->isMeasureRepeat()) {
                        continue;
                    }

                    measureRepeats.push_back(toMeasureRepeat(item));
                }

                handleAnnotations(partId, segment, segmentStartTick);
            }
        }

        handleSpanners(partId, score, repeatSegment->tick,
                       repeatSegment->tick + repeatSegment->len(), tickPositionOffset);

        handleMeasureRepeats(measureRepeats, tickPositionOffset);
    }
}

void PlaybackContext::clear()
{
    m_partStartTrack = 0;
    m_partEndTrack = 0;
    m_dynamicsByTrack.clear();
    m_playTechniquesMap.clear();
    m_playbackParamByTrack.clear();
}

bool PlaybackContext::hasSoundFlags() const
{
    return !m_playbackParamByTrack.empty();
}

dynamic_level_t PlaybackContext::nominalDynamicLevel(const track_idx_t trackIdx, const int positionTick) const
{
    auto dynamicsIt = m_dynamicsByTrack.find(trackIdx);
    if (dynamicsIt == m_dynamicsByTrack.end()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    const DynamicMap& dynamics = dynamicsIt->second;
    auto it = dynamics.find(positionTick);
    if (it == dynamics.end()) {
        return mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
    }

    return it->second.level;
}

void PlaybackContext::updateDynamicMap(const Dynamic* dynamic, const Segment* segment, const int segmentPositionTick)
{
    if (!dynamic->playDynamic()) {
        return;
    }

    const DynamicType type = dynamic->dynamicType();

    if (isOrdinaryDynamicType(type)) {
        applyDynamic(dynamic, dynamicLevelFromType(type), segmentPositionTick);
        return;
    }

    if (isSingleNoteDynamicType(type)) {
        mpe::dynamic_level_t prevDynamicLevel = appliableDynamicLevel(dynamic->track(), segmentPositionTick);
        applyDynamic(dynamic, dynamicLevelFromType(type), segmentPositionTick);

        if (segment->next()) {
            int tickPositionOffset = segmentPositionTick - segment->tick().ticks();
            int nextSegmentPositionTick = segment->next()->tick().ticks() + tickPositionOffset;
            applyDynamic(dynamic, prevDynamicLevel, nextSegmentPositionTick);
        }

        return;
    }

    const DynamicTransition& transition = dynamicTransitionFromType(type);
    const int transitionDuration = dynamic->velocityChangeLength().ticks();

    dynamic_level_t levelFrom = dynamicLevelFromType(transition.from);
    dynamic_level_t levelTo = dynamicLevelFromType(transition.to);

    dynamic_level_t range = levelTo - levelFrom;

    std::map<int, int> dynamicsCurve = TConv::easingValueCurve(transitionDuration,
                                                               6 /*stepsCount*/,
                                                               static_cast<int>(range),
                                                               ChangeMethod::NORMAL);

    for (const auto& pair : dynamicsCurve) {
        applyDynamic(dynamic, levelFrom + pair.second, segmentPositionTick + pair.first);
    }
}

void PlaybackContext::updatePlayTechMap(const PlayTechAnnotation* annotation, const int segmentPositionTick)
{
    const PlayingTechniqueType type = annotation->techniqueType();

    if (type == PlayingTechniqueType::Undefined) {
        return;
    }

    m_playTechniquesMap[segmentPositionTick] = articulationFromPlayTechType(type);

    bool cancelPlayTechniques = type == PlayingTechniqueType::Natural || type == PlayingTechniqueType::Open;

    if (cancelPlayTechniques && !m_playbackParamByTrack.empty()) {
        PlaybackParam ordTechnique(PlaybackParam::PlayingTechnique, mpe::ORDINARY_PLAYING_TECHNIQUE_CODE);

        for (track_idx_t idx = m_partStartTrack; idx < m_partEndTrack; ++idx) {
            m_playbackParamByTrack[idx][segmentPositionTick].push_back(ordTechnique);
        }
    }
}

void PlaybackContext::updatePlaybackParams(const SoundFlagMap& flagsOnSegment, const int segmentPositionTick)
{
    auto trackAccepted = [&flagsOnSegment](const SoundFlag* flag, track_idx_t trackIdx) {
        staff_idx_t staffIdx = track2staff(trackIdx);

        if (flag->staffIdx() == staffIdx) {
            return true;
        }

        if (flag->applyToAllStaves()) {
            return !muse::contains(flagsOnSegment, staffIdx);
        }

        return false;
    };

    for (const auto& pair : flagsOnSegment) {
        const SoundFlag* flag = pair.second;

        for (track_idx_t trackIdx = m_partStartTrack; trackIdx < m_partEndTrack; ++trackIdx) {
            if (!trackAccepted(flag, trackIdx)) {
                continue;
            }

            mpe::PlaybackParamList& params = m_playbackParamByTrack[trackIdx][segmentPositionTick];

            for (const String& soundPreset : flag->soundPresets()) {
                if (!soundPreset.empty()) {
                    params.emplace_back(PlaybackParam::SoundPreset, soundPreset);
                }
            }

            if (!flag->playingTechnique().empty()) {
                params.emplace_back(PlaybackParam::PlayingTechnique, flag->playingTechnique());
            }
        }

        if (flag->playingTechnique() == mpe::ORDINARY_PLAYING_TECHNIQUE_CODE) {
            m_playTechniquesMap[segmentPositionTick] = mpe::ArticulationType::Standard;
        }
    }
}

void PlaybackContext::handleSpanners(const ID partId, const Score* score, const int segmentStartTick, const int segmentEndTick,
                                     const int tickPositionOffset)
{
    const SpannerMap& spannerMap = score->spannerMap();
    if (spannerMap.empty()) {
        return;
    }

    auto intervals = spannerMap.findOverlapping(segmentStartTick, segmentEndTick - 1);
    for (const auto& interval : intervals) {
        const Spanner* spanner = interval.value;

        if (!spanner->isHairpin() || !spanner->playSpanner()) {
            continue;
        }

        if (spanner->part()->id() != partId.toUint64()) {
            continue;
        }

        int spannerFrom = spanner->tick().ticks();
        int spannerTo = spannerFrom + std::abs(spanner->ticks().ticks());

        int spannerDurationTicks = spannerTo - spannerFrom;

        if (spannerDurationTicks <= 0) {
            continue;
        }

        const Hairpin* hairpin = toHairpin(spanner);
        const track_idx_t trackIdx = hairpin->track();

        {
            Segment* startSegment = hairpin->startSegment();
            Dynamic* startDynamic = startSegment
                                    ? toDynamic(startSegment->findAnnotation(ElementType::DYNAMIC, trackIdx, trackIdx))
                                    : nullptr;
            if (startDynamic) {
                if (startDynamic->dynamicType() != DynamicType::OTHER
                    && !isOrdinaryDynamicType(startDynamic->dynamicType())
                    && !isSingleNoteDynamicType(startDynamic->dynamicType())) {
                    // The hairpin starts with a transition dynamic; we should start the hairpin after the transition is complete
                    // This solution should be replaced once we have better infrastructure to see relations between Dynamics and Hairpins.
                    spannerFrom += startDynamic->velocityChangeLength().ticks();

                    spannerDurationTicks = spannerTo - spannerFrom;

                    if (spannerDurationTicks <= 0) {
                        continue;
                    }
                }
            }
        }

        // First, check if hairpin has its own start/end dynamics in the begin/end text
        const DynamicType dynamicTypeFrom = hairpin->dynamicTypeFrom();
        const DynamicType dynamicTypeTo = hairpin->dynamicTypeTo();

        // If it doesn't:
        // - for the start level, use the currently-applicable level at the start tick of the hairpin
        // - for the end level, check if there is a dynamic marking at the end of the hairpin
        const dynamic_level_t levelFrom
            = dynamicLevelFromType(dynamicTypeFrom, appliableDynamicLevel(trackIdx, spannerFrom + tickPositionOffset));
        const dynamic_level_t nominalLevelTo
            = dynamicLevelFromType(dynamicTypeTo, nominalDynamicLevel(trackIdx, spannerTo + tickPositionOffset));

        // If there is an end dynamic marking, check if it matches the 'direction' of the hairpin (cresc. vs decresc.)
        const bool isCrescendo = hairpin->isCrescendo();
        const bool hasNominalLevelTo = nominalLevelTo != mpe::dynamicLevelFromType(mpe::DynamicType::Natural);
        const bool useNominalLevelTo = hasNominalLevelTo && (isCrescendo
                                                             ? nominalLevelTo > levelFrom
                                                             : nominalLevelTo < levelFrom);

        const dynamic_level_t levelTo = useNominalLevelTo
                                        ? nominalLevelTo
                                        : levelFrom + (isCrescendo ? mpe::DYNAMIC_LEVEL_STEP : -mpe::DYNAMIC_LEVEL_STEP);

        std::map<int, int> dynamicsCurve = TConv::easingValueCurve(spannerDurationTicks,
                                                                   24 /*stepsCount*/,
                                                                   static_cast<int>(levelTo - levelFrom),
                                                                   hairpin->veloChangeMethod());

        for (const auto& pair : dynamicsCurve) {
            applyDynamic(hairpin, levelFrom + pair.second, spannerFrom + pair.first + tickPositionOffset);
        }

        if (hasNominalLevelTo && !useNominalLevelTo) {
            // If there is a dynamic at the end of the hairpin that we couldn't use because it didn't match the direction of the hairpin,
            // insert that dynamic directly after the hairpin
            applyDynamic(hairpin, nominalLevelTo, spannerTo + tickPositionOffset);
        }
    }
}

void PlaybackContext::handleAnnotations(const ID partId, const Segment* segment, const int segmentPositionTick)
{
    SoundFlagMap soundFlagsOnSegment;

    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation || !annotation->part()) {
            continue;
        }

        if (annotation->part()->id() != partId.toUint64()) {
            continue;
        }

        if (annotation->isDynamic()) {
            updateDynamicMap(toDynamic(annotation), segment, segmentPositionTick);
            continue;
        }

        if (annotation->isPlayTechAnnotation()) {
            updatePlayTechMap(toPlayTechAnnotation(annotation), segmentPositionTick);
            continue;
        }

        if (annotation->isStaffText()) {
            if (const SoundFlag* flag = toStaffText(annotation)->soundFlag()) {
                if (soundFlagPlayable(flag)) {
                    soundFlagsOnSegment.emplace(flag->staffIdx(), flag);
                }
            }
        }
    }

    if (!soundFlagsOnSegment.empty()) {
        updatePlaybackParams(soundFlagsOnSegment, segmentPositionTick);
    }
}

void PlaybackContext::handleMeasureRepeats(const std::vector<const MeasureRepeat*>& measureRepeats, const int tickPositionOffset)
{
    for (const MeasureRepeat* mr : measureRepeats) {
        const Measure* currMeasure = mr->firstMeasureOfGroup();
        if (!currMeasure) {
            continue;
        }

        const Measure* referringMeasure = mr->referringMeasure(currMeasure);
        if (!referringMeasure) {
            continue;
        }

        int currentMeasureTick = currMeasure->tick().ticks();
        int referringMeasureTick = referringMeasure->tick().ticks();
        int newItemsOffsetTick = currentMeasureTick - referringMeasureTick;

        for (int num = 0; num < mr->numMeasures(); ++num) {
            int startTick = referringMeasure->tick().ticks() + tickPositionOffset;
            int endTick = referringMeasure->endTick().ticks() + tickPositionOffset;

            copyDynamicsInRange(startTick, endTick, newItemsOffsetTick);
            copyPlaybackParamsInRange(startTick, endTick, newItemsOffsetTick);
            copyPlayTechniquesInRange(startTick, endTick, newItemsOffsetTick);

            currMeasure = currMeasure->nextMeasure();
            if (!currMeasure) {
                break;
            }

            referringMeasure = mr->referringMeasure(currMeasure);
            if (!referringMeasure) {
                break;
            }
        }
    }
}

void PlaybackContext::copyDynamicsInRange(const int rangeStartTick, const int rangeEndTick, const int newDynamicsOffsetTick)
{
    for (auto& pair : m_dynamicsByTrack) {
        DynamicMap& dynamics = pair.second;
        auto startIt = dynamics.lower_bound(rangeStartTick);
        if (startIt == dynamics.end()) {
            return;
        }

        auto endIt = dynamics.lower_bound(rangeEndTick);

        DynamicMap newDynamics;
        for (auto it = startIt; it != endIt; ++it) {
            int tick = it->first + newDynamicsOffsetTick;
            newDynamics.insert_or_assign(tick, it->second);
        }

        dynamics.merge(std::move(newDynamics));
    }
}

void PlaybackContext::copyPlaybackParamsInRange(const int rangeStartTick, const int rangeEndTick, const int newParamsOffsetTick)
{
    for (auto& pair : m_playbackParamByTrack) {
        ParamMap& params = pair.second;
        auto startIt = params.lower_bound(rangeStartTick);
        if (startIt == params.end()) {
            return;
        }

        auto endIt = params.lower_bound(rangeEndTick);

        ParamMap newParams;
        for (auto it = startIt; it != endIt; ++it) {
            int tick = it->first + newParamsOffsetTick;
            newParams.insert_or_assign(tick, it->second);
        }

        params.merge(std::move(newParams));
    }
}

void PlaybackContext::copyPlayTechniquesInRange(const int rangeStartTick, const int rangeEndTick, const int newPlayTechOffsetTick)
{
    auto startIt = m_playTechniquesMap.lower_bound(rangeStartTick);
    if (startIt == m_playTechniquesMap.end()) {
        return;
    }

    auto endIt = m_playTechniquesMap.lower_bound(rangeEndTick);

    PlayTechniquesMap newPlayTechniques;
    for (auto it = startIt; it != endIt; ++it) {
        int tick = it->first + newPlayTechOffsetTick;
        newPlayTechniques.insert_or_assign(tick, it->second);
    }

    m_playTechniquesMap.merge(std::move(newPlayTechniques));
}

void PlaybackContext::applyDynamic(const EngravingItem* dynamicItem, const dynamic_level_t dynamicLevel, const int positionTick)
{
    const VoiceAssignment voiceAssignment = dynamicItem->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
    const track_idx_t dynamicTrackIdx = dynamicItem->track();
    const staff_idx_t dynamicStaffIdx = dynamicItem->staffIdx();
    const int dynamicPriority = static_cast<int>(voiceAssignment);

    //! See: https://github.com/musescore/MuseScore/issues/23355
    auto trackAccepted = [voiceAssignment, dynamicTrackIdx, dynamicStaffIdx](const track_idx_t trackIdx) {
        switch (voiceAssignment) {
        case VoiceAssignment::CURRENT_VOICE_ONLY:
            return dynamicTrackIdx == trackIdx;
        case VoiceAssignment::ALL_VOICE_IN_STAFF:
            return dynamicStaffIdx == track2staff(trackIdx);
        case VoiceAssignment::ALL_VOICE_IN_INSTRUMENT:
            return true;
        }

        return false;
    };

    for (track_idx_t trackIdx = m_partStartTrack; trackIdx < m_partEndTrack; ++trackIdx) {
        if (!trackAccepted(trackIdx)) {
            continue;
        }

        DynamicInfo& dynamic = m_dynamicsByTrack[trackIdx][positionTick];
        if (dynamic.priority < dynamicPriority) {
            dynamic.level = dynamicLevel;
            dynamic.priority = dynamicPriority;
        }
    }
}
