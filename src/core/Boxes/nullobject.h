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

#ifndef NULLOBJECT_H
#define NULLOBJECT_H

#include "boundingbox.h"

#include "Properties/comboboxproperty.h"
#include "Animators/coloranimator.h"

class NullObjectType;

class NullObject : public BoundingBox {
public:
    NullObject();

    void queTasks();
    stdsptr<BoxRenderData> createRenderData() { return nullptr; }
    bool shouldScheduleUpdate() { return false; }
    bool relPointInsidePath(const QPointF &relPos) const;

    void drawNullObject(SkCanvas* const canvas, const CanvasMode mode,
                        const float invScale, const bool ctrlPressed);
private:
    qsptr<NullObjectType> mType;
    qsptr<ColorAnimator> mColor;
    qsptr<QrealAnimator> mSize;
};

#endif // NULLOBJECT_H
