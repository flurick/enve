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

#include "svgfilecachehandler.h"

#include "Boxes/containerbox.h"
#include "filesourcescache.h"
#include "GUI/edialogs.h"

SvgFileCacheHandler::SvgFileCacheHandler() {}

void SvgFileCacheHandler::reload() {}

void SvgFileCacheHandler::replace() {
    const QString importPath = eDialogs::openFile(
                "Replace SVG Source " + path(), path(),
                "Files (*.svg)");
    if(!importPath.isEmpty()) {
        const QFile file(importPath);
        if(!file.exists()) return;
        if(hasVectorExt(importPath)) {
            try {
                setPath(importPath);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
    }
}
