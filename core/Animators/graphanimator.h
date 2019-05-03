#ifndef GRAPHANIMATOR_H
#define GRAPHANIMATOR_H
#include "animator.h"
#define GetAsGK(key) GetAsPtr(key, GraphKey)
class GraphKey;
enum QrealPointType : short;
class GraphAnimator : public Animator {
    struct GraphPath {
        operator const QPainterPath&() const { return fPath; }

        QPainterPath fPath;
        int fStartFrame;
        int fEndFrame;
    };

protected:
    GraphAnimator(const QString& name);
public:
    virtual void graph_getValueConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minMoveValue, qreal &maxMoveValue) const = 0;
    virtual qValueRange graph_getMinAndMaxValues() const;
    virtual qValueRange graph_getMinAndMaxValuesBetweenFrames(
            const int &startFrame, const int &endFrame) const;
    virtual qreal graph_clampGraphValue(const qreal &value) { return value; }
    virtual QPainterPath graph_getPathForSegment(
            const GraphKey * const prevKey,
            const GraphKey * const nextKey) const;

    bool SWT_isGraphAnimator() const { return true; }

    void prp_afterChangedAbsRange(const FrameRange& range) {
        Animator::prp_afterChangedAbsRange(range);
        graph_updateKeysPath(prp_absRangeToRelRange(range));
    }
    void graph_constrainCtrlsFrameValues();

    void graph_updateKeysPath(const FrameRange& relFrameRange);

    void graph_addKeysInRectToList(const QRectF &frameValueRect,
                                   QList<GraphKey*> &keys);

    void graph_incSelectedForGraph() {
        graph_mSelected++;
    }

    void graph_decSelectedForGraph() {
        graph_mSelected--;
    }

    bool graph_isSelectedForGraph() {
        return graph_mSelected;
    }

    QColor graph_getAnimatorColor(void *ptr) const {
        for(const auto& x : graph_mAnimatorColors) {
            if(x.first == ptr) {
                return x.second;
            }
        }
        return QColor();
    }

    void graph_setAnimatorColor(void *ptr, const QColor &color) {
        graph_mAnimatorColors[ptr] = color;
    }

    void graph_removeAnimatorColor(void *ptr) {
        graph_mAnimatorColors.erase(ptr);
    }

    bool graph_isCurrentAnimator(void *ptr) const {
        return graph_mAnimatorColors.find(ptr) != graph_mAnimatorColors.end();
    }

    void graph_drawKeysPath(QPainter * const p,
                            const QColor &paintColor,
                            const FrameRange &absFrameRange) const;

    void graph_getFrameValueConstraints(GraphKey *key,
                                  const QrealPointType& type,
                                  qreal &minMoveFrame,
                                  qreal &maxMoveFrame,
                                  qreal &minMoveValue,
                                  qreal &maxMoveValue) const;

    QrealPoint *graph_getPointAt(const qreal &value,
                                 const qreal &frame,
                                 const qreal &pixelsPerFrame,
                                 const qreal &pixelsPerValUnit);
    void graph_changeSelectedKeysFrameAndValueStart(const QPointF &frameVal);
    void graph_changeSelectedKeysFrameAndValue(const QPointF& frameVal);
    void graph_enableCtrlPtsForSelected();
    void graph_setCtrlsModeForSelectedKeys(const CtrlsMode &mode);
    void graph_getSelectedSegments(QList<QList<GraphKey*>> &segments);
protected:
    qreal prevKeyWeight(const GraphKey * const prevKey,
                        const GraphKey * const nextKey,
                        const qreal & frame) const;

    QList<GraphPath> graph_mKeyPaths;
private:
    IdRange graph_relFrameRangeToGraphPathIdRange(
            const FrameRange &relFrameRange) const;

    int idForFrame(const int& relFrame) const {
        return idForFrame(relFrame, 0, graph_mKeyPaths.count() - 1);
    }

    int idForFrame(const int& relFrame,
                   const int& min, const int& max) const {
        if(min >= max) return min;
        const int guess = (max + min)/2;
        const auto& path = graph_mKeyPaths.at(guess);
        if(path.fStartFrame > relFrame) {
            return idForFrame(relFrame, min, guess - 1);
        } else if(path.fEndFrame < relFrame) {
            return idForFrame(relFrame, guess + 1, max);
        }
        // guessFrame == relFrame
        return guess;
    }

    void graph_getFrameConstraints(
            GraphKey *key, const QrealPointType& type,
            qreal &minMoveFrame, qreal &maxMoveFrame) const;

    int graph_mSelected = 0;

    std::map<void*, QColor> graph_mAnimatorColors;
};

#endif // GRAPHANIMATOR_H
