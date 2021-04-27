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

#ifndef TRACKEFFECT_H
#define TRACKEFFECT_H

#include "targettransformeffect.h"

class TrackEffect : public TargetTransformEffect {
public:
    TrackEffect();

    void applyEffect(const qreal relFrame,
                     qreal &pivotX, qreal &pivotY,
                     qreal &posX, qreal &posY,
                     qreal &rot,
                     qreal &scaleX, qreal &scaleY,
                     qreal &shearX, qreal &shearY,
                     QMatrix& postTransform,
                     BoundingBox* const parent) override;
private:
    void setRotScaleAfterTargetChange(
                BoundingBox* const oldTarget,
                BoundingBox* const newTarget) override;

    qsptr<QrealAnimator> mInfluence;
};

#endif // TRACKEFFECT_H
