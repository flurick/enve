#include "paintsettingsanimator.h"
#include "undoredo.h"
#include "colorhelpers.h"
#include "PropertyUpdaters/displayedfillstrokesettingsupdater.h"
#include "skia/skqtconversions.h"
#include "skia/skiaincludes.h"
#include "Boxes/pathbox.h"
#include "gradientpoints.h"
#include "gradient.h"
#include "document.h"

PaintSettingsAnimator::PaintSettingsAnimator(
        const QString &name,
        GradientPoints * const grdPts,
        PathBox * const parent) :
    PaintSettingsAnimator(name, grdPts, parent, QColor(255, 255, 255),
                          PaintType::FLATPAINT,  nullptr) {}

PaintSettingsAnimator::PaintSettingsAnimator(
        const QString& name,
        GradientPoints * const grdPts,
        PathBox * const parent,
        const QColor &colorT,
        const PaintType paintTypeT,
        Gradient* const gradientT) :
    ComplexAnimator(name), mTarget_k(parent) {
    mColor->qra_setCurrentValue(colorT);
    showHideChildrenBeforeChaningPaintType(paintTypeT);
    mPaintType = paintTypeT;
    setGradientVar(gradientT);
    setGradientPoints(grdPts);

    prp_setOwnUpdater(enve::make_shared<DisplayedFillStrokeSettingsUpdater>(parent));
}

void PaintSettingsAnimator::writeProperty(eWriteStream& dst) const {
    mGradientPoints->writeProperty(dst);
    mColor->writeProperty(dst);
    dst.write(rcConstChar(&mPaintType), sizeof(PaintType));
    dst.write(rcConstChar(&mGradientType), sizeof(GradientType));
    const int gradId = mGradient ? mGradient->getReadWriteId() : -1;
    dst << gradId;
}

void PaintSettingsAnimator::readProperty(eReadStream& src) {
    mGradientPoints->readProperty(src);
    mColor->readProperty(src);
    PaintType paintType;
    src.read(rcChar(&paintType), sizeof(PaintType));
    int gradId;
    src.read(rcChar(&mGradientType), sizeof(GradientType));
    src >> gradId;
    if(gradId != -1) {
        mGradient = Document::sInstance->getGradientWithRWId(gradId);
    }
    setPaintType(paintType);
}

void PaintSettingsAnimator::setGradientVar(Gradient* const grad) {
    if(grad == mGradient) return;
    if(mGradient) {
        ca_removeChild(mGradient->ref<Gradient>());
        ca_removeChild(mGradientPoints->ref<GradientPoints>());
        disconnect(mGradient, &Gradient::changed, this, nullptr);
    }
    if(grad) {
        ca_addChild(grad->ref<Gradient>());
        ca_addChild(mGradientPoints->ref<GradientPoints>());
        connect(grad, &Gradient::changed,
                this, [this]() { mTarget_k->updateDrawGradients(); });
        if(grad && !mGradient) {
            if(mTarget_k->getFillSettings() == this)
                mTarget_k->resetFillGradientPointsPos();
            else mTarget_k->resetFillGradientPointsPos();
        }
    }
    mGradient = grad;

    prp_callFinishUpdater();
}

QColor PaintSettingsAnimator::getColor() const {
    return mColor->getColor();
}

QColor PaintSettingsAnimator::getColor(const qreal relFrame) const {
    return mColor->getColor(relFrame);
}

PaintType PaintSettingsAnimator::getPaintType() const {
    return mPaintType;
}

Gradient *PaintSettingsAnimator::getGradient() const {
    return mGradient.data();
}

void PaintSettingsAnimator::setGradient(Gradient* gradient) {
    if(gradient == mGradient) return;
    setGradientVar(gradient);
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
}

void PaintSettingsAnimator::setCurrentColor(const QColor &color) {
    mColor->qra_setCurrentValue(color);
}

void PaintSettingsAnimator::showHideChildrenBeforeChaningPaintType(
        const PaintType newPaintType) {
    if(mPaintType == FLATPAINT)
        ca_removeChild(mColor);
    if(newPaintType == FLATPAINT)
        ca_addChild(mColor);
}

void PaintSettingsAnimator::setPaintType(const PaintType paintType) {
    if(paintType == mPaintType) return;

    showHideChildrenBeforeChaningPaintType(paintType);

    mPaintType = paintType;
    mTarget_k->updateDrawGradients();
    mTarget_k->requestGlobalFillStrokeUpdateIfSelected();
    prp_callFinishUpdater();
}

ColorAnimator *PaintSettingsAnimator::getColorAnimator() {
    return mColor.data();
}

void PaintSettingsAnimator::setGradientPoints(GradientPoints* const gradientPoints) {
    mGradientPoints = gradientPoints;
}

void PaintSettingsAnimator::setGradientPointsPos(const QPointF &pt1, const QPointF &pt2) {
    if(!mGradientPoints) return;
    mGradientPoints->setPositions(pt1, pt2);
}

UpdatePaintSettings::UpdatePaintSettings(const QColor &paintColorT,
                                         const PaintType paintTypeT) {
    fPaintColor = paintColorT;
    fPaintType = paintTypeT;
}

UpdatePaintSettings::UpdatePaintSettings() {}

UpdatePaintSettings::~UpdatePaintSettings() {}

void UpdatePaintSettings::applyPainterSettingsSk(SkPaint *paint) {
    if(fPaintType == GRADIENTPAINT) {
        //p->setBrush(gradient);
        paint->setShader(fGradient);
        paint->setAlpha(255);
    } else if(fPaintType == FLATPAINT) {
        paint->setColor(toSkColor(fPaintColor));
    } else {
        paint->setColor(SkColorSetARGB(0, 0, 0, 0));
    }
}

void UpdatePaintSettings::updateGradient(const QGradientStops &stops,
                                         const QPointF &start,
                                         const QPointF &finalStop,
                                         const GradientType gradientType) {
    int nStops = stops.count();
    SkPoint gradPoints[nStops];
    SkColor gradColors[nStops];
    float gradPos[nStops];
    const float xInc = static_cast<float>(finalStop.x() - start.x());
    const float yInc = static_cast<float>(finalStop.y() - start.y());
    float currX = static_cast<float>(start.x());
    float currY = static_cast<float>(start.y());
    float currT = 0;
    const float tInc = 1.f/(nStops - 1);

    for(int i = 0; i < nStops; i++) {
        const QGradientStop &stopT = stops.at(i);
        const QColor col = stopT.second;
        gradPoints[i] = SkPoint::Make(currX, currY);
        gradColors[i] = toSkColor(col);
        gradPos[i] = currT;

        currX += xInc;
        currY += yInc;
        currT += tInc;
    }
    if(gradientType == GradientType::LINEAR) {
        fGradient = SkGradientShader::MakeLinear(gradPoints, gradColors,
                                                 gradPos, nStops,
                                                 SkTileMode::kClamp);
    } else {
        const QPointF distPt = finalStop - start;
        const qreal radius = qSqrt(pow2(distPt.x()) + pow2(distPt.y()));
        fGradient = SkGradientShader::MakeRadial(
                        toSkPoint(start), toSkScalar(radius),
                        gradColors, gradPos,
                        nStops, SkTileMode::kClamp);
    }
}

UpdateStrokeSettings::UpdateStrokeSettings(const qreal width,
                   const QColor &paintColorT,
                   const PaintType paintTypeT,
                   const QPainter::CompositionMode &outlineCompositionModeT) :
    UpdatePaintSettings(paintColorT, paintTypeT), fOutlineWidth(width) {
    fOutlineCompositionMode = outlineCompositionModeT;
}

UpdateStrokeSettings::UpdateStrokeSettings() {}

void UpdateStrokeSettings::applyPainterSettingsSk(SkPaint *paint) {
    UpdatePaintSettings::applyPainterSettingsSk(paint);
}
