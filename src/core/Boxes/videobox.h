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

#ifndef VIDEOBOX_H
#define VIDEOBOX_H
#include <QString>
#include <unordered_map>
#include "Boxes/animationbox.h"
#include "FileCacheHandlers/videocachehandler.h"

class eVideoSound;

class CORE_EXPORT VideoBox : public AnimationBox {
    e_OBJECT
protected:
    VideoBox();

    void prp_readPropertyXEV_impl(const QDomElement& ele, const XevImporter& imp);
    QDomElement prp_writePropertyXEV_impl(const XevExporter& exp) const;
public:
    void changeSourceFile();

    void writeBoundingBox(eWriteStream& dst) const;
    void readBoundingBox(eReadStream& src);

    void setStretch(const qreal stretch);

    eVideoSound* sound() const
    { return mSound.get(); }
    void setFilePath(const QString& path);
    QString getFilePath();
private:
    void setFilePathNoRename(const QString &path);

    void soundDataChanged();
    void fileHandlerConnector(ConnContext& conn, VideoFileHandler* obj);
    void fileHandlerAfterAssigned(VideoFileHandler* obj);

    qsptr<eVideoSound> mSound;
    FileHandlerObjRef<VideoFileHandler> mFileHandler;
};

#endif // VIDEOBOX_H
