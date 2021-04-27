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

#ifndef COLORPICKINGWIDGET_H
#define COLORPICKINGWIDGET_H

#include <QWidget>

class ColorSettingsWidget;

class ColorPickingWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColorPickingWidget(QScreen* const screen,
                                QWidget * const parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void focusOutEvent(QFocusEvent*);
signals:
    void colorSelected(QColor);
private:
    QColor colorFromPoint(const int x, const int y);
    void updateBox(const QPoint &pos);

    QImage mScreenshot;
    QColor mCurrentColor;
    int mCursorX = 0;
    int mCursorY = 0;
};

#endif // COLORPICKINGWIDGET_H
