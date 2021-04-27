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

#ifndef XEVEXPORTER_H
#define XEVEXPORTER_H

#include "../smartPointers/stdselfref.h"

#include "xmlexporthelpers.h"
#include "runtimewriteid.h"

class XevZipFileSaver;

class XevExporter : public StdSelfRef {
public:
    XevExporter(QDomDocument& doc,
                const std::shared_ptr<XevZipFileSaver>& xevFileSaver,
                const RuntimeIdToWriteId& objListIdConv,
                const QString& path,
                const QString& assetsPath = "");

    const RuntimeIdToWriteId& objListIdConv() const { return mObjectListIdConv; }

    QDomDocument& doc() const { return mDoc; }

    stdsptr<XevExporter> withAssetsPath(const QString& path) const;

    QDomElement createElement(const QString& tagName) const;
    QDomText createTextNode(const QString& data) const;

    using Processor = std::function<void(QIODevice* const dst)>;
    void processAsset(const QString& file, const Processor& func,
                      const bool compress = true) const;

    QString absPathToRelPath(const QString& absPath) const;
private:
    QDomDocument& mDoc;
    const stdsptr<XevZipFileSaver> mFileSaver;
    const RuntimeIdToWriteId& mObjectListIdConv;
    const QString mPath;
    const QString mAssetsPath;
};

#endif // XEVEXPORTER_H
