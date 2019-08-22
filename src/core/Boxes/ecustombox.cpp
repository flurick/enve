#include "ecustombox.h"

eCustomBox::eCustomBox(const QString& name) :
    BoundingBox(eBoxType::TYPE_CUSTOM) {
    prp_setName(name);
}

void eCustomBox::writeIdentifier(eWriteStream& dst) const {
    BoundingBox::writeIdentifier(dst);
    getIdentifier().write(dst);
}
