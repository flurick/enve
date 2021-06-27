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

#ifndef SCENESETTINGSDIALOG_H
#define SCENESETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include "smartPointers/ememory.h"
#include "framerange.h"


class Document;
class Canvas;
class ColorAnimator;
class ColorAnimatorButton;

class SceneSettingsDialog : public QDialog {
public:
    SceneSettingsDialog(Canvas * const canvas,
                         QWidget * const parent = nullptr);
    SceneSettingsDialog(const QString &defName,
                         QWidget * const parent = nullptr);
    SceneSettingsDialog(const QString &name,
                         const int width,
                         const int height,
                         const FrameRange &range,
                         const qreal fps,
                         ColorAnimator * const bg,
                         QWidget * const parent = nullptr);

    int getCanvasWidth() const;
    int getCanvasHeight() const;
    QString getCanvasName() const;
    FrameRange getFrameRange() const;
    qreal getFps() const;

    void applySettingsToCanvas(Canvas * const canvas) const;

    static void sNewSceneDialog(Document &document, QWidget * const parent);
private:
    bool validate();

    Canvas * mTargetCanvas = nullptr;

    QVBoxLayout *mMainLayout;

    QHBoxLayout *mNameLayout;
    QLabel *mNameEditLabel;
    QLineEdit *mNameEdit;

    QHBoxLayout *mSizeLayout;
    QLabel *mWidthLabel;
    QSpinBox *mWidthSpinBox;
    QLabel *mHeightLabel;
    QSpinBox *mHeightSpinBox;

    QHBoxLayout *mFrameRangeLayout;
    QLabel *mFrameRangeLabel;
    QSpinBox *mMinFrameSpin;
    QSpinBox *mMaxFrameSpin;
    QComboBox *mTypeTime;

    QHBoxLayout *mFPSLayout;
    QLabel *mFPSLabel;
    QDoubleSpinBox *mFPSSpinBox;

    QHBoxLayout *mBgColorLayout;
    QLabel *mBgColorLabel;
    ColorAnimatorButton *mBgColorButton;

    QLabel* mErrorLabel;

    QPushButton *mOkButton;
    QPushButton *mCancelButton;
    QHBoxLayout *mButtonsLayout;
};

#endif // SCENESETTINGSDIALOG_H
