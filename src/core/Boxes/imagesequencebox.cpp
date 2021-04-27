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

#include "imagesequencebox.h"
#include "FileCacheHandlers/imagesequencecachehandler.h"
#include "filesourcescache.h"
#include "GUI/edialogs.h"
#include "fileshandler.h"
#include "Boxes/containerbox.h"

ImageSequenceFileHandler* imageSequenceFileHandlerGetter(const QString& path) {
    const auto fileHandler = FilesHandler::sInstance;
    return fileHandler->getFileHandler<ImageSequenceFileHandler>(path);
}

ImageSequenceBox::ImageSequenceBox() :
    AnimationBox("Image Sequence", eBoxType::imageSequence),
    mFileHandler(this,
                 [](const QString& path) {
                     return imageSequenceFileHandlerGetter(path);
                 },
                 [this](ImageSequenceFileHandler* obj) {
                     return fileHandlerAfterAssigned(obj);
                 },
                 [this](ConnContext& conn, ImageSequenceFileHandler* obj) {
                     fileHandlerConnector(conn, obj);
                 }) {}

void ImageSequenceBox::fileHandlerConnector(ConnContext &conn,
                                            ImageSequenceFileHandler *obj) {
    Q_UNUSED(conn)
    Q_UNUSED(obj)
}

void ImageSequenceBox::fileHandlerAfterAssigned(ImageSequenceFileHandler *obj) {
    qsptr<ImageSequenceCacheHandler> frameHandler;
    if(obj) frameHandler = enve::make_shared<ImageSequenceCacheHandler>(obj);
    setAnimationFramesHandler(frameHandler);
    animationDataChanged();
}

void ImageSequenceBox::setFolderPath(const QString &folderPath) {
    mFileHandler.assign(folderPath);
}

void ImageSequenceBox::changeSourceFile() {
    const auto dir = eDialogs::openDir("Import Image Sequence",
                                       mFileHandler.path());
    if(!dir.isEmpty()) setFolderPath(dir);
}

void ImageSequenceBox::writeBoundingBox(eWriteStream& dst) const {
    AnimationBox::writeBoundingBox(dst);
    dst.writeFilePath(mFileHandler->path());
}

void ImageSequenceBox::readBoundingBox(eReadStream& src) {
    AnimationBox::readBoundingBox(src);
    const QString path = src.readFilePath();
    setFolderPath(path);
}

QDomElement ImageSequenceBox::prp_writePropertyXEV_impl(
        const XevExporter& exp) const {
    auto result = AnimationBox::prp_writePropertyXEV_impl(exp);
    const QString& absSrc = mFileHandler.path();
    XevExportHelpers::setAbsAndRelFileSrc(absSrc, result, exp);
    return result;
}

void ImageSequenceBox::prp_readPropertyXEV_impl(
        const QDomElement& ele, const XevImporter& imp) {
    AnimationBox::prp_readPropertyXEV_impl(ele, imp);
    const QString absSrc = XevExportHelpers::getAbsAndRelFileSrc(ele, imp);
    setFolderPath(absSrc);
}
