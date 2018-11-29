#ifndef BRUSHSETTINGWIDGET_H
#define BRUSHSETTINGWIDGET_H

#include <QObject>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include "../PaintLib/brush.h"
#include <QPushButton>

struct WindowVariables;

class BrushSettingWidget : public QObject
{
    Q_OBJECT
public:
    explicit BrushSettingWidget(QVBoxLayout *label_layout_t,
                                QVBoxLayout *rest_layout_t,
                                BrushSetting setting_id_t,
                                QWidget *parent = nullptr);
    qreal getVal();
    void setVal(const qreal &val_t,
                const bool &edited_t);

    void valChanged();

    void hide();
    void show();
    void setVisible(const bool &b_t);

    void setDefaultButtonEnabled(const bool &b_t);
signals:
    void setBrushSetting(BrushSetting, qreal);
public slots:
    void setSpinVal(const int &val_t);
    void setSliderVal(double val_t);
    void incVal(const qreal &inc_t);
    void revertToDefaultVal();
private:
    QHBoxLayout *mMainLayout = nullptr;
    QPushButton *mDefaultButton = nullptr;
    const BrushSettingInfo *mSettingInfo = nullptr;
    QLabel *mNameLabel = nullptr;
    QDoubleSpinBox *mValSpinBox = nullptr;
    QSlider *mValSlider = nullptr;
    BrushSetting mTargetSetting;
};

#endif // BRUSHSETTINGWIDGET_H