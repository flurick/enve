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

#include "xevzipfilesaver.h"

XevZipFileSaver::XevZipFileSaver() {}

void XevZipFileSaver::setPath(const QString& path) {
    mDir.setPath(QFileInfo(path).path());
    mFile.setFileName(path);
    if(mFile.exists()) mFile.remove();
    if(!mFile.open(QIODevice::WriteOnly))
        RuntimeThrow("Could not open file for writing '" + path + "'.");
    mFileSaver.setIoDevice(&mFile);
}

QString XevZipFileSaver::absPathToRelPath(const QString& absPath) const {
    return mDir.relativeFilePath(absPath);
}
