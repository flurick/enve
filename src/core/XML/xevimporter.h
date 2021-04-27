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

#ifndef XEVIMPORTER_H
#define XEVIMPORTER_H

#include "xmlexporthelpers.h"

#include "runtimewriteid.h"

class ZipFileLoader;
class BoundingBox;

class CORE_EXPORT XevReadBoxesHandler {
public:
    ~XevReadBoxesHandler();

    void addReadBox(const int readId, BoundingBox * const box);
    BoundingBox *getBoxByReadId(const int readId) const;
    using XevImporterDoneTask = std::function<void(const XevReadBoxesHandler&)>;
    void addXevImporterDoneTask(const XevImporterDoneTask& task);
private:
    std::map<int, BoundingBox*> mReadBoxes;
    QList<XevImporterDoneTask> mDoneTasks;

};

class XevImporter {
public:
    XevImporter(XevReadBoxesHandler& xevReadBoxesHandler,
                ZipFileLoader& fileLoader,
                const RuntimeIdToWriteId& objListIdConv,
                const QString& path,
                const QString& assetsPath = "");

    XevReadBoxesHandler& getXevReadBoxesHandler() const {
        return mXevReadBoxesHandler;
    }

    const RuntimeIdToWriteId& objListIdConv() const { return mObjectListIdConv; }

    XevImporter withAssetsPath(const QString& path) const;

    using Processor = std::function<void(QIODevice* const dst)>;
    void processAsset(const QString& file, const Processor& func) const;

    QString relPathToAbsPath(const QString& relPath) const;
private:
    XevReadBoxesHandler& mXevReadBoxesHandler;
    ZipFileLoader& mFileLoader;
    const RuntimeIdToWriteId& mObjectListIdConv;
    const QString mPath;
    const QString mAssetsPath;
};

#endif // XEVIMPORTER_H
