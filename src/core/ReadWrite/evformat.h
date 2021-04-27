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

#ifndef EVFORMAT_H
#define EVFORMAT_H

namespace EvFormat {
    enum {
        dataCompression = 16,
        textSkFont = 17,
        oilEffectImprov = 18,
        betterSWTAbsReadWrite = 19,
        readSceneSettingsBeforeContent = 20,
        relativeFilePathSave = 21,
        flipBook = 22,
        colorizeInfluence = 23,
        transformEffects = 24,
        transformEffects2 = 25,

        nextVersion
    };

    const int version = nextVersion - 1;
}

#endif // EVFORMAT_H
