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

#ifndef XEVZIPFILESAVER_H
#define XEVZIPFILESAVER_H

#include <QDir>

#include "zipfilesaver.h"

class CORE_EXPORT XevZipFileSaver {
public:
    XevZipFileSaver();

    ZipFileSaver& fileSaver() { return mFileSaver; }

    void setPath(const QString& path);

    QString absPathToRelPath(const QString& absPath) const;
private:
    QDir mDir;
    QFile mFile;
    ZipFileSaver mFileSaver;
};

#endif // XEVZIPFILESAVER_H
