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

#ifndef EXTERNALLINKBOX_H
#define EXTERNALLINKBOX_H

#include "FileCacheHandlers/filehandlerobjref.h"
#include "boundingbox.h"
#include "typemenu.h"

template <class T, class FHandler>
class ExternalLinkBoxT : public T {
protected:
    using FHandlerRef = FileHandlerObjRef<FHandler>;
    ExternalLinkBoxT(const typename FHandlerRef::Getter& getter,
                     const typename FHandlerRef::AfterAssigned& afterAssigned = nullptr,
                     const typename FHandlerRef::Connector& connector = nullptr) :
        mFileHandler(this, getter, afterAssigned, connector) {
        this->prp_setName("Empty Link");
    }

    QDomElement prp_writePropertyXEV_impl(const XevExporter& exp) const override {
        auto result = T::prp_writePropertyXEV(exp);
        XevExportHelpers::setAbsAndRelFileSrc(getFilePath(), result, exp);
        return result;
    }

    void prp_readPropertyXEV_impl(const QDomElement& ele,
                             const XevImporter& imp) override {
        T::prp_readPropertyXEV_impl(ele, imp);
        const QString absSrc = XevExportHelpers::getAbsAndRelFileSrc(ele, imp);
        setFilePath(absSrc);
    }
public:
    virtual void changeSourceFile() = 0;

    void writeBoundingBox(eWriteStream& dst) const override {
        BoundingBox::writeBoundingBox(dst);
        dst.writeFilePath(getFilePath());
    }

    void readBoundingBox(eReadStream& src) override {
        BoundingBox::readBoundingBox(src);
        const QString path = src.readFilePath();
        setFilePath(path);
    }

    void setupCanvasMenu(PropertyMenu * const menu) override;

    bool isLink() const final { return true; }

    void reload() {
        const auto fileHandler = this->fileHandler();
        if(fileHandler) fileHandler->reloadAction();
    }

    void setFilePath(const QString& path)
    { mFileHandler.assign(path); }
    const QString& getFilePath() const
    { return mFileHandler.path(); }
protected:
    FHandler* fileHandler() const { return mFileHandler.data(); }
private:
    FHandlerRef mFileHandler;
};

template <class T, class FHandler>
void ExternalLinkBoxT<T, FHandler>::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<ExternalLinkBoxT>()) return;
    menu->addedActionsForType<ExternalLinkBoxT>();

    const PropertyMenu::PlainSelectedOp<ExternalLinkBoxT> reloadOp =
    [](ExternalLinkBoxT * box) {
        box->reload();
    };
    menu->addPlainAction("Reload", reloadOp);

    const PropertyMenu::PlainSelectedOp<ExternalLinkBoxT> setSrcOp =
    [](ExternalLinkBoxT * box) {
        box->changeSourceFile();
    };
    menu->addPlainAction("Set Source File...", setSrcOp);

    T::setupCanvasMenu(menu);
}

#endif // EXTERNALLINKBOX_H
