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

#ifndef STATICCOMPLEXANIMATOR_H
#define STATICCOMPLEXANIMATOR_H
#include "complexanimator.h"

class CORE_EXPORT StaticComplexAnimator : public ComplexAnimator {
public:
    StaticComplexAnimator(const QString &name);

    void prp_writeProperty(eWriteStream& dst) const;
    void prp_readProperty(eReadStream& src);

    void prp_readPropertyXEV(const QDomElement& ele, const XevImporter& imp);
    QDomElement prp_writePropertyXEV(const XevExporter& exp) const;

    using ComplexAnimator::ca_addChild;
protected:
    void writeChildPropertiesXEV(QDomElement& prop, const XevExporter& exp) const;
private:
    using ComplexAnimator::ca_removeAllChildren;
    using ComplexAnimator::ca_removeChild;
    using ComplexAnimator::ca_replaceChild;
    using ComplexAnimator::ca_takeChildAt;
};

#endif // STATICCOMPLEXANIMATOR_H
