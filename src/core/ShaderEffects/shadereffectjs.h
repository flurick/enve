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

#ifndef SHADEREFFECTJS_H
#define SHADEREFFECTJS_H

#include "../core_global.h"

#include "skia/skiaincludes.h"

#include <memory>
#include <QJSEngine>

class CORE_EXPORT ShaderEffectJS {
public:
    struct Blueprint;
    ShaderEffectJS(const Blueprint& blueprint);

    struct GlValueBlueprint {
        QString fName;
        QString fScript;
    };

    struct Blueprint {
        static std::shared_ptr<Blueprint> sCreate(
                const QString& defs, QString calc,
                const QStringList& properties,
                const QList<GlValueBlueprint>& glValueBPs,
                const QString& marginScript);

        const QString fClassDef;
        const QStringList fGlValues;
        const bool fMargin;
    };

    void setValues(const QJSValueList& args);

    void evaluate();

    int glValueCount() const;
    QJSValue getGlValue(const int index);

    QJSValue& getGlValueGetter(const int index);

    const bool fMargin;
    QJSValue getMarginValue();

    void setSceneRect(const SkIRect& rect);

    QJSValue toValue(const QPointF& val);
    QJSValue toValue(const QColor& val);
private:
    QJSEngine mEngine;
    QJSValue m_eSetSceneRect;
    QJSValue m_eSet;
    QJSValue m_eEvaluate;
    QJSValueList mGlValueGetters;
    QJSValue mMarginGetter;
};

#endif // SHADEREFFECTJS_H
