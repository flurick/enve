// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "targettransformeffect.h"

#include "Boxes/boundingbox.h"
#include "Animators/transformanimator.h"

TargetTransformEffect::TargetTransformEffect(
        const QString& name,
        const TransformEffectType type) :
    TransformEffect(name, type) {
    mTarget = enve::make_shared<BoxTargetProperty>("target");

    connect(mTarget.get(), &BoxTargetProperty::targetSet,
            this, [this](BoundingBox* const newTarget) {
        auto& conn = mTargetConn.assign(newTarget);
        if(newTarget) {
            const auto parent = getFirstAncestor<BoundingBox>();
            const auto parentTransform = parent->getTransformAnimator();
            const auto targetTransform = newTarget->getTransformAnimator();
            conn << connect(targetTransform, &Property::prp_absFrameRangeChanged,
                            this, [parentTransform](const FrameRange& range,
                            const bool clip) {
                parentTransform->prp_afterChangedAbsRange(range, clip);
            });
            conn << connect(targetTransform,
                            &AdvancedTransformAnimator::totalTransformChanged,
                            this, [parentTransform](const UpdateReason reason) {
                parentTransform->prp_afterChangedCurrent(reason);
            });
            conn << connect(parentTransform,
                            &AdvancedTransformAnimator::inheritedTransformChanged,
                            this, [parentTransform](const UpdateReason reason) {
                parentTransform->prp_afterChangedCurrent(reason);
            });
        }
    });

    ca_addChild(mTarget);
}

BoxTargetProperty* TargetTransformEffect::targetProperty() {
    return mTarget.get();
}
