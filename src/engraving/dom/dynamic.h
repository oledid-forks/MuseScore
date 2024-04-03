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

#ifndef MU_ENGRAVING_DYNAMICS_H
#define MU_ENGRAVING_DYNAMICS_H

#include "textbase.h"

namespace mu::engraving {
class Measure;
class Segment;

struct Dyn {
    DynamicType type = DynamicType::OTHER;
    int velocity = -1;              // associated midi velocity (0-127, -1 = none)
    int changeInVelocity = 0;
    bool accent = 0;                // if true add velocity to current chord velocity
    const char* text = nullptr;     // utf8 text of dynamic
};

//-----------------------------------------------------------------------------
//   @@ Dynamic
///    dynamics marker; determines midi velocity
//
//   @P range  enum (Dynamic.STAFF, .PART, .SYSTEM)
//-----------------------------------------------------------------------------

class Dynamic final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, Dynamic)
    DECLARE_CLASSOF(ElementType::DYNAMIC)

public:
    struct ChangeSpeedItem {
        DynamicSpeed speed = DynamicSpeed::NORMAL;
        const char* name = nullptr;
    };

    Dynamic(Segment* parent);
    Dynamic(const Dynamic&);
    Dynamic* clone() const override { return new Dynamic(*this); }
    Segment* segment() const { return (Segment*)explicitParent(); }
    Measure* measure() const { return (Measure*)explicitParent()->explicitParent(); }

    void setDynamicType(DynamicType val) { m_dynamicType = val; }
    void setDynamicType(const String&);

    DynamicType dynamicType() const { return m_dynamicType; }
    int subtype() const override { return static_cast<int>(m_dynamicType); }
    TranslatableString subtypeUserName() const override;
    String translatedSubtypeUserName() const override;

    double customTextOffset() const;

    bool isEditable() const override { return true; }
    bool isEditAllowed(EditData&) const override;
    void startEdit(EditData&) override;
    bool edit(EditData&) override;
    bool editNonTextual(EditData&) override;
    void editDrag(EditData&) override;
    void endEdit(EditData&) override;
    void reset() override;
    bool needStartEditingAfterSelecting() const override { return true; }

    bool allowTimeAnchor() const override { return true; }

    void setVelocity(int v) { m_velocity = v; }
    int velocity() const;
    DynamicRange dynRange() const { return m_dynRange; }
    void setDynRange(DynamicRange t) { m_dynRange = t; }
    void undoSetDynRange(DynamicRange t);

    int changeInVelocity() const;
    void setChangeInVelocity(int val);
    Fraction velocityChangeLength() const;
    bool isVelocityChangeAvailable() const;

    DynamicSpeed velChangeSpeed() const { return m_velChangeSpeed; }
    void setVelChangeSpeed(DynamicSpeed val) { m_velChangeSpeed = val; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;
    void undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps) override;

    std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const EngravingItem*)> isDragged) override;

    String accessibleInfo() const override;
    String screenReaderInfo() const override;
//    void doAutoplace();
    void manageBarlineCollisions();

    static String dynamicText(DynamicType t);
    bool hasCustomText() const { return dynamicText(m_dynamicType) != xmlText(); }

    void setSnappedExpression(Expression* e) { m_snappedExpression = e; }
    Expression* snappedExpression() const { return m_snappedExpression; }

    bool playDynamic() const { return m_playDynamic; }
    void setPlayDynamic(bool v) { m_playDynamic = v; }

    bool acceptDrop(EditData& ed) const override;
    EngravingItem* drop(EditData& ed) override;

    static const std::vector<Dyn>& dynamicList() { return DYN_LIST; }

    bool anchorToEndOfPrevious() const { return m_anchorToEndOfPrevious; }
    void setAnchorToEndOfPrevious(bool v) { m_anchorToEndOfPrevious = v; }

private:

    M_PROPERTY(bool, avoidBarLines, setAvoidBarLines)
    M_PROPERTY(double, dynamicsSize, setDynamicsSize)
    M_PROPERTY(bool, centerOnNotehead, setCenterOnNotehead)

    bool changeTimeAnchorType(const EditData& ed);
    bool moveSegment(const EditData& ed);
    bool nudge(const EditData& ed);

    DynamicType m_dynamicType = DynamicType::OTHER;
    Expression* m_snappedExpression = nullptr;
    bool m_playDynamic = true;

    mutable mu::PointF m_dragOffset;
    int m_velocity = -1;           // associated midi velocity 0-127
    DynamicRange m_dynRange = DynamicRange::PART; // STAFF, PART, SYSTEM

    int m_changeInVelocity = 128;
    DynamicSpeed m_velChangeSpeed = DynamicSpeed::NORMAL;

    static const std::vector<Dyn> DYN_LIST;

    bool m_anchorToEndOfPrevious = false;
};
} // namespace mu::engraving

#endif
