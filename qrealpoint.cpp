#include "qrealpoint.h"
#include "qrealkey.h"

QrealPoint::QrealPoint(QrealPointType type, QrealKey *parentKey, qreal radius) {
    mRadius = radius;
    mType = type;
    mParentKey = parentKey;
}

qreal QrealPoint::getFrame() {
    if(mType == KEY_POINT) return mParentKey->getAbsFrame();
    if(mType == START_POINT) return mParentKey->getStartValueFrame();
    if(mType == END_POINT) return mParentKey->getEndValueFrame();
}

void QrealPoint::setFrame(qreal frame)
{
    if(mType == KEY_POINT) return mParentKey->setFrameAndUpdateParentAnimator(
                                                                qRound(frame));
    if(mType == START_POINT) return mParentKey->setStartFrame(frame);
    if(mType == END_POINT) return mParentKey->setEndFrame(frame);
}

qreal QrealPoint::getValue() {
    if(mType == KEY_POINT) return mParentKey->getValue();
    if(mType == START_POINT) return mParentKey->getStartValue();
    if(mType == END_POINT) return mParentKey->getEndValue();
}

void QrealPoint::setValue(qreal value, bool finish)
{
    if(mType == KEY_POINT) return mParentKey->setValue(value, finish);
    if(mType == START_POINT) return mParentKey->setStartValue(value);
    if(mType == END_POINT) return mParentKey->setEndValue(value);
}

bool QrealPoint::isSelected() {
    if(mType == KEY_POINT) return mParentKey->isSelected();
    return mIsSelected;
}

bool QrealPoint::isNear(const qreal &frameT,
                        const qreal &valueT,
                        const qreal &pixelsPerFrame,
                        const qreal &pixelsPerValue) {
    qreal value = getValue();
    qreal frame = getFrame();
    if(qAbs(frameT - frame)*pixelsPerFrame > mRadius) return false;
    if(qAbs(valueT - value)*pixelsPerValue > mRadius) return false;
    return true;
}

void QrealPoint::moveTo(qreal frameT, qreal valueT) {
    setFrame(frameT);
    setValue(valueT);
    if(isKeyPoint() ) return;
    mParentKey->updateCtrlFromCtrl(mType);
}

void QrealPoint::draw(QPainter *p, const QColor &paintColor,
                      const qreal &minFrameT, const qreal &minValueT,
                      const qreal &pixelsPerFrame, const qreal &pixelsPerValue) {
    QPointF center = QPointF((getFrame() - minFrameT + 0.5)*pixelsPerFrame,
                             (minValueT - getValue())*pixelsPerValue);
    p->setBrush(Qt::black);
    p->drawEllipse(center, mRadius, mRadius);

    p->setBrush(paintColor);
    if(isSelected()) {
        p->drawEllipse(center, mRadius - 1., mRadius - 1.);
    } else {
        p->drawEllipse(center, mRadius*0.5 - 1., mRadius*0.5 - 1.);
    }
}

void QrealPoint::setSelected(bool bT) {
    if(mType == KEY_POINT) {
        mParentKey->setSelected(bT);
    } else {
        mIsSelected = bT;
    }
}

bool QrealPoint::isKeyPoint() { return mType == KEY_POINT; }

bool QrealPoint::isStartPoint() { return mType == START_POINT; }

bool QrealPoint::isEndPoint() { return mType == END_POINT; }

bool QrealPoint::isEnabled() {
    if(isKeyPoint() ) return true;
    if(isStartPoint() ) return mParentKey->isStartPointEnabled();
    if(isEndPoint() ) return mParentKey->isEndPointEnabled();
}

QrealKey *QrealPoint::getParentKey()
{
    return mParentKey;
}
