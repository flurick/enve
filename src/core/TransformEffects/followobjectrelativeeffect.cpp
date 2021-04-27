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

#include "followobjectrelativeeffect.h"

#include "Boxes/boundingbox.h"
#include "Animators/transformanimator.h"
#include "Animators/qrealanimator.h"

FollowObjectRelativeEffect::FollowObjectRelativeEffect() :
    FollowObjectEffectBase("follow object relative",
                          TransformEffectType::followObjectRelative) {
}

void FollowObjectRelativeEffect::applyEffect(
        const qreal relFrame,
        qreal& pivotX, qreal& pivotY,
        qreal& posX, qreal& posY,
        qreal& rot,
        qreal& scaleX, qreal& scaleY,
        qreal& shearX, qreal& shearY,
        QMatrix& postTransform,
        BoundingBox* const parent) {
    Q_UNUSED(pivotX)
    Q_UNUSED(pivotY)
    Q_UNUSED(shearX)
    Q_UNUSED(shearY)
    Q_UNUSED(postTransform)

    if(!isVisible()) return;

    if(!parent) return;
    const auto target = targetProperty()->getTarget();
    if(!target) return;

    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal targetRelFrame = target->prp_absFrameToRelFrameF(absFrame);

    const auto targetTransform = target->getTotalTransformAtFrame(targetRelFrame);

    applyEffectWithTransform(relFrame, pivotX, pivotY,
                             posX, posY, rot,
                             scaleX, scaleY, shearX, shearY,
                             parent, targetTransform);
}
