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

#ifndef COLORIZEEFFECT_H
#define COLORIZEEFFECT_H

#include "rastereffect.h"

class ColorizeEffect : public RasterEffect {
public:
    ColorizeEffect();

    stdsptr<RasterEffectCaller> getEffectCaller(
            const qreal relFrame, const qreal resolution,
            const qreal influence, BoxRenderData * const data) const;
private:
    qsptr<QrealAnimator> mInfluence;
    qsptr<QrealAnimator> mHue;
    qsptr<QrealAnimator> mSaturation;
    qsptr<QrealAnimator> mLightness;
};

#endif // COLORIZEEFFECT_H
