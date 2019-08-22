#ifndef TIMELINEWRAPPER_H
#define TIMELINEWRAPPER_H
#include "canvaswindowwrapper.h"
class TimelineWidget;
class ChangeWidthWidget;

struct TWidgetStackLayoutItem : public SceneWidgetStackLayoutItem {
    TWidgetStackLayoutItem() {}
    void clear();
    QWidget* create(Document &document,
                    QWidget * const parent,
                    QLayout* const layout = nullptr);
    void write(eWriteStream &dst) const;
    void read(eReadStream& src);
    void setGraph(const bool graph);
private:
    bool mGraph = false;
};

class TimelineWrapper : public StackWidgetWrapper {
public:
    TimelineWrapper(Document * const document,
                    TWidgetStackLayoutItem * const layItem,
                    QWidget * const parent = nullptr);

    void setScene(Canvas* const scene);
    Canvas* getScene() const;
    TimelineWidget *getTimelineWidget() const;

    void saveDataToLayout() const;
private:
    using StackWidgetWrapper::setMenuBar;
    using StackWidgetWrapper::setCentralWidget;
};

#endif // TIMELINEWRAPPER_H
