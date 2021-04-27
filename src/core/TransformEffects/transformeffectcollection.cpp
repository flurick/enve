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

#include "transformeffectcollection.h"
#include "ReadWrite/evformat.h"

#include "trackeffect.h"
#include "followpatheffect.h"
#include "followobjecteffect.h"
#include "followobjectrelativeeffect.h"
#include "parenteffect.h"

TransformEffectCollection::TransformEffectCollection() :
    TransformEffectCollectionBase("transform effects") {}

void TransformEffectCollection::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<TransformEffectCollection>()) return;
    menu->addedActionsForType<TransformEffectCollection>();
    {
        const PropertyMenu::PlainSelectedOp<TransformEffectCollection> aOp =
        [](TransformEffectCollection * coll) {
            coll->addChild(enve::make_shared<TrackEffect>());
        };
        menu->addPlainAction("Add Track Effect", aOp);
    }
    {
        const PropertyMenu::PlainSelectedOp<TransformEffectCollection> aOp =
        [](TransformEffectCollection * coll) {
            coll->addChild(enve::make_shared<FollowPathEffect>());
        };
        menu->addPlainAction("Add Follow Path Effect", aOp);
    }
    {
        const PropertyMenu::PlainSelectedOp<TransformEffectCollection> aOp =
        [](TransformEffectCollection * coll) {
            coll->addChild(enve::make_shared<FollowObjectEffect>());
        };
        menu->addPlainAction("Add Follow Object Effect", aOp);
    }
    {
        const PropertyMenu::PlainSelectedOp<TransformEffectCollection> aOp =
        [](TransformEffectCollection * coll) {
            coll->addChild(enve::make_shared<FollowObjectRelativeEffect>());
        };
        menu->addPlainAction("Add Follow Object Relative Effect", aOp);
    }
    {
        const PropertyMenu::PlainSelectedOp<TransformEffectCollection> aOp =
        [](TransformEffectCollection * coll) {
            coll->addChild(enve::make_shared<ParentEffect>());
        };
        menu->addPlainAction("Add Parent Effect", aOp);
    }
    menu->addSeparator();
    TransformEffectCollection::prp_setupTreeViewMenu(menu);
}

void TransformEffectCollection::prp_readProperty(eReadStream &src) {
    if(src.evFileVersion() < EvFormat::transformEffects) return;
    TransformEffectCollectionBase::prp_readProperty(src);
}

void TransformEffectCollection::prp_readProperty_impl(eReadStream &src) {
    if(src.evFileVersion() == EvFormat::transformEffects) {
        TransformEffectCollectionBase::prp_readProperty_impl(src);
        bool visible; src >> visible;
    } else {
        TransformEffectCollectionBase::prp_readProperty_impl(src);
    }
}

void TransformEffectCollection::applyEffects(
        const qreal relFrame,
        qreal& pivotX, qreal& pivotY,
        qreal& posX, qreal& posY,
        qreal& rot,
        qreal& scaleX, qreal& scaleY,
        qreal& shearX, qreal& shearY,
        QMatrix& postTransform,
        BoundingBox* const parent) const {
    const int iMax = ca_getNumberOfChildren();
    for(int i = 0; i < iMax; i++) {
        const auto effect = getChild(i);
        if(!effect->isVisible()) continue;
        effect->applyEffect(relFrame,
                            pivotX, pivotY,
                            posX, posY,
                            rot,
                            scaleX, scaleY,
                            shearX, shearY,
                            postTransform,
                            parent);
    }
}

qsptr<TransformEffect> createTransformEffectForType(
        const TransformEffectType type) {
    switch(type) {
        case(TransformEffectType::track):
            return enve::make_shared<TrackEffect>();
        case(TransformEffectType::followPath):
            return enve::make_shared<FollowPathEffect>();
        case(TransformEffectType::followObject):
            return enve::make_shared<FollowObjectEffect>();
        case(TransformEffectType::followObjectRelative):
            return enve::make_shared<FollowObjectRelativeEffect>();
        case(TransformEffectType::parent):
            return enve::make_shared<ParentEffect>();
        default: RuntimeThrow("Invalid transform effect type '" +
                              QString::number(int(type)) + "'");
    }
}

qsptr<TransformEffect> readIdCreateTransformEffect(eReadStream &src) {
    TransformEffectType type;
    src.read(&type, sizeof(TransformEffectType));
    return createTransformEffectForType(type);
}

void writeTransformEffectType(TransformEffect * const obj, eWriteStream &dst) {
    obj->writeIdentifier(dst);
}

qsptr<TransformEffect> readIdCreateTransformEffectXEV(const QDomElement& ele) {
    const QString typeStr = ele.attribute("type");
    const int typeInt = XmlExportHelpers::stringToInt(typeStr);
    const auto type = static_cast<TransformEffectType>(typeInt);
    return createTransformEffectForType(type);
}

void writeTransformEffectTypeXEV(TransformEffect* const obj, QDomElement& ele) {
    obj->writeIdentifierXEV(ele);
}
