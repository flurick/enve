#ifndef CUSTOMRASTEREFFECT_H
#define CUSTOMRASTEREFFECT_H
#include "rastereffect.h"
#include "../customidentifier.h"

class CustomRasterEffect : public RasterEffect {
protected:
    CustomRasterEffect(const QString& name,
                       const HardwareSupport hwSupport,
                       const bool hwInterchangeable);
public:
    virtual CustomIdentifier getIdentifier() const = 0;
private:
    void writeIdentifier(eWriteStream &dst) const final;
};

#endif // CUSTOMRASTEREFFECT_H
