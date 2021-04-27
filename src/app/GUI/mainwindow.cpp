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

#include "mainwindow.h"
#include "canvas.h"
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include <QStatusBar>
#include "usagewidget.h"
#include <QToolBar>
#include "GUI/ColorWidgets/colorsettingswidget.h"
#include <QMenuBar>
#include "GUI/edialogs.h"
#include <QMessageBox>
#include "timelinedockwidget.h"
#include "Private/Tasks/taskexecutor.h"
#include "qdoubleslider.h"
#include "canvaswindow.h"
#include "GUI/BoxesList/boxscrollwidget.h"
#include "clipboardcontainer.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "GUI/BoxesList/boxscroller.h"
#include "GUI/RenderWidgets/renderwidget.h"
#include "actionbutton.h"
#include "fontswidget.h"
#include "GUI/global.h"
#include "filesourcescache.h"
#include "filesourcelist.h"
#include "videoencoder.h"
#include "fillstrokesettings.h"
#include <QAudioOutput>
#include "Sound/soundcomposition.h"
#include "GUI/BoxesList/boxsinglewidget.h"
#include "memoryhandler.h"
#include "GUI/BrushWidgets/brushselectionwidget.h"
#include "Animators/gradient.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include "GUI/Dialogs/scenesettingsdialog.h"
#include "ShaderEffects/shadereffectprogram.h"
#include "importhandler.h"
#include "envesplash.h"
#include "envelicense.h"
#include "switchbutton.h"
#include "centralwidget.h"
#include "ColorWidgets/bookmarkedcolors.h"
#include "GUI/edialogs.h"
#include "GUI/dialogsinterface.h"
#include "closesignalingdockwidget.h"
#include "eimporters.h"
#include "ColorWidgets/paintcolorwidget.h"
#include "Dialogs/exportsvgdialog.h"
#include "alignwidget.h"

MainWindow *MainWindow::sInstance = nullptr;

void MainWindow::keyPressEvent(QKeyEvent *event) {
    processKeyEvent(event);
}

MainWindow::MainWindow(Document& document,
                       Actions& actions,
                       AudioHandler& audioHandler,
                       RenderHandler& renderHandler,
                       QWidget * const parent)
    : QMainWindow(parent),
      mDocument(document),
      mActions(actions),
      mAudioHandler(audioHandler),
      mRenderHandler(renderHandler) {
    Q_ASSERT(!sInstance);
    sInstance = this;

    ImportHandler::sInstance->addImporter<eXevImporter>();
    ImportHandler::sInstance->addImporter<evImporter>();
    ImportHandler::sInstance->addImporter<eSvgImporter>();
    ImportHandler::sInstance->addImporter<eOraImporter>();
    ImportHandler::sInstance->addImporter<eKraImporter>();

    connect(&mDocument, &Document::evFilePathChanged,
            this, &MainWindow::updateTitle);
    connect(&mDocument, &Document::documentChanged,
            this, [this]() {
        setFileChangedSinceSaving(true);
    });
    connect(&mDocument, &Document::activeSceneSet,
            this, &MainWindow::updateSettingsForCurrentCanvas);
    connect(&mDocument, &Document::currentBoxChanged,
            this, &MainWindow::setCurrentBox);
    connect(&mDocument, &Document::canvasModeSet,
            this, &MainWindow::updateCanvasModeButtonsChecked);
    connect(&mDocument, &Document::sceneCreated,
            this, &MainWindow::closeWelcomeDialog);

    const auto iconDir = eSettings::sIconsDir();
    setWindowIcon(QIcon(iconDir + "/enve.png"));
    const auto downArr = iconDir + "/down-arrow.png";
    const auto upArr = iconDir + "/up-arrow.png";
    const auto dockClose = iconDir + "/dockClose.png";
    const auto dockMaximize = iconDir + "/dockMaximize.png";

    const QString iconSS =
            "QComboBox::down-arrow { image: url(" + downArr + "); }"
            "QScrollBar::sub-line { image: url(" + upArr + "); }"
            "QScrollBar::add-line { image: url(" + downArr + "); }"
            "QDockWidget {"
                "titlebar-close-icon: url(" + dockClose + ");"
                "titlebar-normal-icon: url(" + dockMaximize + ");"
            "}";

    QFile customSS(eSettings::sSettingsDir() + "/stylesheet.qss");
    if(customSS.exists()) {
        if(customSS.open(QIODevice::ReadOnly | QIODevice::Text)) {
            setStyleSheet(customSS.readAll());
            customSS.close();
        }
    } else {
        QFile file(":/styles/stylesheet.qss");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            setStyleSheet(file.readAll() + iconSS);
            file.close();
        }
    }
    BoxSingleWidget::loadStaticPixmaps();

    BrushSelectionWidget::sPaintContext = BrushSelectionWidget::sCreateNewContext();
    BrushSelectionWidget::sOutlineContext = BrushSelectionWidget::sCreateNewContext();

//    for(int i = 0; i < ClipboardContainerType::CCT_COUNT; i++) {
//        mClipboardContainers << nullptr;
//    }

    mDocument.setPath("");

    mFillStrokeSettingsDock = new CloseSignalingDockWidget(
                tr("Fill and Stroke", "Dock"), this);
    //const auto fillStrokeSettingsScroll = new ScrollArea(this);
    mFillStrokeSettings = new FillStrokeSettingsWidget(mDocument, this);
    //fillStrokeSettingsScroll->setWidget(mFillStrokeSettings);
    mFillStrokeSettingsDock->setWidget(mFillStrokeSettings);
    addDockWidget(Qt::RightDockWidgetArea, mFillStrokeSettingsDock);
    eSizesUI::widget.add(mFillStrokeSettingsDock, [this](const int size) {
        mFillStrokeSettingsDock->setMinimumWidth(size*12);
        mFillStrokeSettingsDock->setMaximumWidth(size*20);
    });

    mPaintColorWidget = new PaintColorWidget(this);
    mPaintColorWidget->hide();
    connect(mPaintColorWidget, &PaintColorWidget::colorChanged,
            &mDocument, &Document::setBrushColor);
    connect(&mDocument, &Document::brushColorChanged,
            mPaintColorWidget, &PaintColorWidget::setDisplayedColor);

    mTimelineDock = new CloseSignalingDockWidget(tr("Timeline", "Dock"), this);
    mTimelineDock->setTitleBarWidget(new QWidget());
    addDockWidget(Qt::BottomDockWidgetArea, mTimelineDock);

    mLayoutHandler = new LayoutHandler(mDocument, mAudioHandler, this);
    mTimeline = new TimelineDockWidget(mDocument, mLayoutHandler, this);
    mTimelineDock->setWidget(mTimeline);

    mBrushSettingsDock = new CloseSignalingDockWidget(
                tr("Brush Settings", "Dock"), this);
    eSizesUI::widget.add(mBrushSettingsDock, [this](const int size) {
        mBrushSettingsDock->setMinimumWidth(size*10);
        mBrushSettingsDock->setMaximumWidth(size*20);
    });

    const auto pCtxt = BrushSelectionWidget::sPaintContext;
    mBrushSelectionWidget = new BrushSelectionWidget(*pCtxt.get(), this);
    connect(mBrushSelectionWidget, &BrushSelectionWidget::currentBrushChanged,
            &mDocument, &Document::setBrush);
//    connect(mBrushSettingsWidget,
//            SIGNAL(brushReplaced(const Brush*,const Brush*)),
//            mCanvasWindow,
//            SLOT(replaceBrush(const Brush*,const Brush*)));

    mBrushSettingsDock->setWidget(mBrushSelectionWidget);

    addDockWidget(Qt::LeftDockWidgetArea, mBrushSettingsDock);
    mBrushSettingsDock->hide();

    mAlignDock = new CloseSignalingDockWidget(
                tr("Align", "Dock"), this);

    const auto alignWidget = new AlignWidget(this);
    connect(alignWidget, &AlignWidget::alignTriggered,
            this, [this](const Qt::Alignment align,
                         const AlignPivot pivot,
                         const AlignRelativeTo relativeTo) {
        const auto scene = *mDocument.fActiveScene;
        if(!scene) return;
        scene->alignSelectedBoxes(align, pivot, relativeTo);
        mDocument.actionFinished();
    });
    mAlignDock->setWidget(alignWidget);

    addDockWidget(Qt::RightDockWidgetArea, mAlignDock);
    mAlignDock->hide();

    mSelectedObjectDock = new CloseSignalingDockWidget(
                tr("Selected Objects", "Dock"), this);
    eSizesUI::widget.add(mSelectedObjectDock, [this](const int size) {
        mSelectedObjectDock->setMinimumWidth(size*10);
        mSelectedObjectDock->setMaximumWidth(size*20);
    });

    mObjectSettingsScrollArea = new ScrollArea(this);
    mObjectSettingsWidget = new BoxScrollWidget(
                mDocument, mObjectSettingsScrollArea);
    mObjectSettingsScrollArea->setWidget(mObjectSettingsWidget);
    mObjectSettingsWidget->setCurrentRule(SWT_BoxRule::selected);
    mObjectSettingsWidget->setCurrentTarget(nullptr, SWT_Target::group);

    connect(mObjectSettingsScrollArea->verticalScrollBar(),
            &QScrollBar::valueChanged,
            mObjectSettingsWidget, &BoxScrollWidget::changeVisibleTop);
    connect(mObjectSettingsScrollArea, &ScrollArea::heightChanged,
            mObjectSettingsWidget, &BoxScrollWidget::changeVisibleHeight);
    connect(mObjectSettingsScrollArea, &ScrollArea::widthChanged,
            mObjectSettingsWidget, &BoxScrollWidget::setWidth);

    const auto vBar = mObjectSettingsScrollArea->verticalScrollBar();
    eSizesUI::widget.add(vBar, [vBar](const int size) {
        vBar->setSingleStep(size);
    });

    mSelectedObjectDock->setWidget(mObjectSettingsScrollArea);
    addDockWidget(Qt::LeftDockWidgetArea, mSelectedObjectDock);

    mFilesDock = new CloseSignalingDockWidget(tr("Files", "Dock"), this);
    eSizesUI::widget.add(mFilesDock, [this](const int size) {
        mFilesDock->setMinimumWidth(size*10);
        mFilesDock->setMaximumWidth(size*20);
    });

    mFilesDock->setWidget(new FileSourceList(this));
    addDockWidget(Qt::LeftDockWidgetArea, mFilesDock);


    {
        const auto brush = BrushCollectionData::sGetBrush("Deevad", "2B_pencil");
        const auto paintCtxt = BrushSelectionWidget::sPaintContext;
        const auto paintBrush = paintCtxt->brushWrapper(brush);
        mDocument.setBrush(paintBrush);
        mDocument.fOutlineBrush = brush;
    }
    const auto bBrush = new BookmarkedBrushes(true, 64, pCtxt.get(), this);
    const auto bColor = new BookmarkedColors(true, 64, this);

    mCentralWidget = new CentralWidget(bBrush,
                                       mLayoutHandler->sceneLayout(),
                                       bColor);

    setupToolBar();
    setupStatusBar();
    setupMenuBar();

    connectToolBarActions();

    readRecentFiles();
    updateRecentMenu();

    mEventFilterDisabled = false;

    installEventFilter(this);

    openWelcomeDialog();
    showMaximized();
}

MainWindow::~MainWindow() {
    sInstance = nullptr;
//    mtaskExecutorThread->terminate();
//    mtaskExecutorThread->quit();
    BoxSingleWidget::clearStaticPixmaps();
}

#include "noshortcutaction.h"
#include "efiltersettings.h"
#include "Settings/settingsdialog.h"
void MainWindow::setupMenuBar() {
    mMenuBar = new QMenuBar(nullptr);
    connectAppFont(mMenuBar);

    mFileMenu = mMenuBar->addMenu(tr("File", "MenuBar"));
    mFileMenu->addAction(tr("New...", "MenuBar_File"),
                         this, &MainWindow::newFile,
                         Qt::CTRL + Qt::Key_N);
    mFileMenu->addAction(tr("Open...", "MenuBar_File"),
                         this, qOverload<>(&MainWindow::openFile),
                         Qt::CTRL + Qt::Key_O);
    mRecentMenu = mFileMenu->addMenu(tr("Open Recent", "MenuBar_File"));
    mFileMenu->addSeparator();
    mFileMenu->addAction("Link...",
                         this, &MainWindow::linkFile,
                         Qt::CTRL + Qt::Key_L);
    mFileMenu->addAction(tr("Import File...", "MenuBar_File"),
                         this, qOverload<>(&MainWindow::importFile),
                         Qt::CTRL + Qt::Key_I);
    mFileMenu->addAction(tr("Import Image Sequence...", "MenuBar_File"),
                         this, &MainWindow::importImageSequence);
    mFileMenu->addSeparator();
    mFileMenu->addAction(tr("Revert", "MenuBar_File"),
                         this, &MainWindow::revert);
    mFileMenu->addSeparator();
    mFileMenu->addAction(tr("Save", "MenuBar_File"),
                         this, qOverload<>(&MainWindow::saveFile),
                         Qt::CTRL + Qt::Key_S);
    mFileMenu->addAction(tr("Save As...", "MenuBar_File"),
                         this, [this]() { saveFileAs(); },
                         Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    mFileMenu->addAction(tr("Save Backup", "MenuBar_File"),
                         this, &MainWindow::saveBackup);

    const auto exportMenu = mFileMenu->addMenu(tr("Export", "MenuBar_File"));
    exportMenu->addAction(tr("Export SVG...", "MenuBar_File"),
                          this, &MainWindow::exportSVG);
    mFileMenu->addSeparator();
    mFileMenu->addAction(tr("Close", "MenuBar_File"),
                         this, &MainWindow::closeProject);
    mFileMenu->addSeparator();
    mFileMenu->addAction(tr("Exit", "MenuBar_File"),
                         this, &MainWindow::close);

    mEditMenu = mMenuBar->addMenu(tr("Edit", "MenuBar"));

    const auto undoQAct = mEditMenu->addAction(tr("Undo", "MenuBar_Edit"));
    undoQAct->setShortcut(Qt::CTRL + Qt::Key_Z);
    mActions.undoAction->connect(undoQAct);

    const auto redoQAct = mEditMenu->addAction(tr("Redo", "MenuBar_Edit"));
    redoQAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
    mActions.redoAction->connect(redoQAct);

    mEditMenu->addSeparator();

    {
        const auto qAct = new NoShortcutAction(tr("Copy", "MenuBar_Edit"));
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_C);
        mActions.copyAction->connect(qAct);
    }

    {
        const auto qAct = new NoShortcutAction(tr("Cut", "MenuBar_Edit"));
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_X);
        mActions.cutAction->connect(qAct);
    }

    {
        const auto qAct = new NoShortcutAction(tr("Paste", "MenuBar_Edit"));
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_V);
        mActions.pasteAction->connect(qAct);
    }

    mEditMenu->addSeparator();

    {
        const auto qAct = new NoShortcutAction(tr("Duplicate", "MenuBar_Edit"));
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::CTRL + Qt::Key_D);
        mActions.duplicateAction->connect(qAct);
    }

    mEditMenu->addSeparator();

    {
        const auto qAct = new NoShortcutAction(tr("Delete", "MenuBar_Edit"));
        mEditMenu->addAction(qAct);
        qAct->setShortcut(Qt::Key_Delete);
        mActions.deleteAction->connect(qAct);
    }

    mEditMenu->addSeparator();
    mEditMenu->addAction(new NoShortcutAction(
                             tr("Select All", "MenuBar_Edit"),
                             &mActions, &Actions::selectAllAction,
                             Qt::Key_A, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction(
                             tr("Invert Selection", "MenuBar_Edit"),
                             &mActions, &Actions::invertSelectionAction,
                             Qt::Key_I, mEditMenu));
    mEditMenu->addAction(new NoShortcutAction(
                             tr("Clear Selection", "MenuBar_Edit"),
                             &mActions, &Actions::clearSelectionAction,
                             Qt::ALT + Qt::Key_A, mEditMenu));
    mEditMenu->addSeparator();
    mEditMenu->addAction(tr("Settings...", "MenuBar_Edit"), [this]() {
        const auto settDial = new SettingsDialog(this);
        settDial->setAttribute(Qt::WA_DeleteOnClose);
        settDial->show();
    });


//    mSelectSameMenu = mEditMenu->addMenu("Select Same");
//    mSelectSameMenu->addAction("Fill and Stroke");
//    mSelectSameMenu->addAction("Fill Color");
//    mSelectSameMenu->addAction("Stroke Color");
//    mSelectSameMenu->addAction("Stroke Style");
//    mSelectSameMenu->addAction("Object Type");

    mObjectMenu = mMenuBar->addMenu(tr("Object", "MenuBar"));

    mObjectMenu->addSeparator();

    const auto raiseQAct = mObjectMenu->addAction(
                tr("Raise", "MenuBar_Object"));
    raiseQAct->setShortcut(Qt::Key_PageUp);
    mActions.raiseAction->connect(raiseQAct);

    const auto lowerQAct = mObjectMenu->addAction(
                tr("Lower", "MenuBar_Object"));
    lowerQAct->setShortcut(Qt::Key_PageDown);
    mActions.lowerAction->connect(lowerQAct);

    const auto rttQAct = mObjectMenu->addAction(
                tr("Raise to Top", "MenuBar_Object"));
    rttQAct->setShortcut(Qt::Key_Home);
    mActions.raiseToTopAction->connect(rttQAct);

    const auto ltbQAct = mObjectMenu->addAction(
                tr("Lower to Bottom", "MenuBar_Object"));
    ltbQAct->setShortcut(Qt::Key_End);
    mActions.lowerToBottomAction->connect(ltbQAct);

    mObjectMenu->addSeparator();

    {
        const auto qAct = mObjectMenu->addAction(
                    tr("Rotate 90° CW", "MenuBar_Object"));
        mActions.rotate90CWAction->connect(qAct);
    }

    {
        const auto qAct = mObjectMenu->addAction(
                    tr("Rotate 90° CCW", "MenuBar_Object"));
        mActions.rotate90CCWAction->connect(qAct);
    }

    {
        const auto qAct = mObjectMenu->addAction(
                    tr("Flip Horizontal", "MenuBar_Object"));
        qAct->setShortcut(Qt::Key_H);
        mActions.flipHorizontalAction->connect(qAct);
    }

    {
        const auto qAct = mObjectMenu->addAction(
                    tr("Flip Vertical", "MenuBar_Object"));
        qAct->setShortcut(Qt::Key_V);
        mActions.flipVerticalAction->connect(qAct);
    }

    mObjectMenu->addSeparator();

    const auto groupQAct = mObjectMenu->addAction(
                tr("Group", "MenuBar_Object"));
    groupQAct->setShortcut(Qt::CTRL + Qt::Key_G);
    mActions.groupAction->connect(groupQAct);

    const auto ungroupQAct = mObjectMenu->addAction(
                tr("Ungroup", "MenuBar_Object"));
    ungroupQAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);
    mActions.ungroupAction->connect(ungroupQAct);

    mObjectMenu->addSeparator();

    const auto transformMenu = mObjectMenu->addMenu(
                tr("Transform", "MenuBar_Object"));
    const auto moveAct = transformMenu->addAction(
                tr("Move", "MenuBar_Object_Transform"));
    moveAct->setShortcut(Qt::Key_G);
    moveAct->setDisabled(true);
    const auto rotateAct = transformMenu->addAction(
                tr("Rotate", "MenuBar_Object_Transform"));
    rotateAct->setShortcut(Qt::Key_R);
    rotateAct->setDisabled(true);
    const auto scaleAct = transformMenu->addAction(
                tr("Scale", "MenuBar_Object_Transform"));
    scaleAct->setShortcut(Qt::Key_S);
    scaleAct->setDisabled(true);
    transformMenu->addSeparator();
    const auto xAct = transformMenu->addAction(
                tr("X-Axis Only", "MenuBar_Object_Transform"));
    xAct->setShortcut(Qt::Key_X);
    xAct->setDisabled(true);
    const auto yAct = transformMenu->addAction(
                tr("Y-Axis Only", "MenuBar_Object_Transform"));
    yAct->setShortcut(Qt::Key_Y);
    yAct->setDisabled(true);


    mPathMenu = mMenuBar->addMenu(tr("Path", "MenuBar"));

    const auto otpQAct = mPathMenu->addAction(
                tr("Object to Path", "MenuBar_Path"));
    mActions.objectsToPathAction->connect(otpQAct);

    const auto stpQAct = mPathMenu->addAction(
                tr("Stroke to Path", "MenuBar_Path"));
    mActions.strokeToPathAction->connect(stpQAct);

    mPathMenu->addSeparator();

    {
        const auto qAct = mPathMenu->addAction(
                    tr("Union", "MenuBar_Path"));
        qAct->setShortcut(Qt::CTRL + Qt::Key_Plus);
        mActions.pathsUnionAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction(
                    tr("Difference", "MenuBar_Path"));
        qAct->setShortcut(Qt::CTRL + Qt::Key_Minus);
        mActions.pathsDifferenceAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction(
                    tr("Intersection", "MenuBar_Path"));
        qAct->setShortcut(Qt::CTRL + Qt::Key_Asterisk);
        mActions.pathsIntersectionAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction(
                    tr("Exclusion", "MenuBar_Path"));
        qAct->setShortcut(Qt::CTRL + Qt::Key_AsciiCircum);
        mActions.pathsExclusionAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction(
                    tr("Division", "MenuBar_Path"));
        qAct->setShortcut(Qt::CTRL + Qt::Key_Slash);
        mActions.pathsDivisionAction->connect(qAct);
    }


//    mPathMenu->addAction("Cut Path", mCanvas,
//                         &Actions::pathsCutAction);

    mPathMenu->addSeparator();

    {
        const auto qAct = mPathMenu->addAction(
                    tr("Combine", "MenuBar_Path"));
        qAct->setShortcut(Qt::CTRL + Qt::Key_K);
        mActions.pathsCombineAction->connect(qAct);
    }

    {
        const auto qAct = mPathMenu->addAction(
                    tr("Break Apart", "MenuBar_Path"));
        qAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_K);
        mActions.pathsBreakApartAction->connect(qAct);
    }

//    mEffectsMenu = mMenuBar->addMenu("Effects");

//    mEffectsMenu->addAction("Blur");

    mSceneMenu = mMenuBar->addMenu(tr("Scene", "MenuBar"));

    mSceneMenu->addAction(tr("New Scene...", "MenuBar_Scene"), this, [this]() {
        SceneSettingsDialog::sNewSceneDialog(mDocument, this);
    });

    {
        const auto qAct = mSceneMenu->addAction(
                    tr("Delete Scene", "MenuBar_Scene"));
        mActions.deleteSceneAction->connect(qAct);
    }

    mSceneMenu->addSeparator();

    mSceneMenu->addAction(tr("Add to Render Queue", "MenuBar_Scene"),
                          this, &MainWindow::addCanvasToRenderQue);

    mSceneMenu->addSeparator();

    {
        const auto qAct = mSceneMenu->addAction(
                    tr("Settings...", "MenuBar_Scene"));
        mActions.sceneSettingsAction->connect(qAct);
    }

    mViewMenu = mMenuBar->addMenu(tr("View", "MenuBar"));

    const auto filteringMenu = mViewMenu->addMenu(
                tr("Filtering", "MenuBar_View"));

    mNoneQuality = filteringMenu->addAction(
                tr("None", "MenuBar_View_Filtering"), [this]() {
        eFilterSettings::sSetDisplayFilter(kNone_SkFilterQuality);
        centralWidget()->update();

        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mNoneQuality->setCheckable(true);
    mNoneQuality->setChecked(eFilterSettings::sDisplay() == kNone_SkFilterQuality &&
                             !eFilterSettings::sSmartDisplat());

    mLowQuality = filteringMenu->addAction(
                tr("Low", "MenuBar_View_Filtering"), [this]() {
        eFilterSettings::sSetDisplayFilter(kLow_SkFilterQuality);
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mLowQuality->setCheckable(true);
    mLowQuality->setChecked(eFilterSettings::sDisplay() == kLow_SkFilterQuality &&
                            !eFilterSettings::sSmartDisplat());

    mMediumQuality = filteringMenu->addAction(
                tr("Medium", "MenuBar_View_Filtering"), [this]() {
        eFilterSettings::sSetDisplayFilter(kMedium_SkFilterQuality);
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mLowQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mMediumQuality->setCheckable(true);
    mMediumQuality->setChecked(eFilterSettings::sDisplay() == kMedium_SkFilterQuality &&
                               !eFilterSettings::sSmartDisplat());

    mHighQuality = filteringMenu->addAction(
                tr("High", "MenuBar_View_Filtering"), [this]() {
        eFilterSettings::sSetDisplayFilter(kHigh_SkFilterQuality);
        centralWidget()->update();

        mNoneQuality->setChecked(false);
        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mDynamicQuality->setChecked(false);
    });
    mHighQuality->setCheckable(true);
    mHighQuality->setChecked(eFilterSettings::sDisplay() == kHigh_SkFilterQuality &&
                             !eFilterSettings::sSmartDisplat());

    mDynamicQuality = filteringMenu->addAction(
                tr("Dynamic", "MenuBar_View_Filtering"), [this]() {
        eFilterSettings::sSetSmartDisplay(true);
        centralWidget()->update();

        mLowQuality->setChecked(false);
        mMediumQuality->setChecked(false);
        mHighQuality->setChecked(false);
        mNoneQuality->setChecked(false);
    });
    mDynamicQuality->setCheckable(true);
    mDynamicQuality->setChecked(eFilterSettings::sSmartDisplat());

    mClipViewToCanvas = mViewMenu->addAction(
                tr("Clip to Scene", "MenuBar_View"));
    mClipViewToCanvas->setCheckable(true);
    mClipViewToCanvas->setChecked(true);
    mClipViewToCanvas->setShortcut(QKeySequence(Qt::Key_C));
    connect(mClipViewToCanvas, &QAction::triggered,
            &mActions, &Actions::setClipToCanvas);

    mRasterEffectsVisible = mViewMenu->addAction(
                tr("Raster Effects", "MenuBar_View"));
    mRasterEffectsVisible->setCheckable(true);
    mRasterEffectsVisible->setChecked(true);
    connect(mRasterEffectsVisible, &QAction::triggered,
            &mActions, &Actions::setRasterEffectsVisible);

    mPathEffectsVisible = mViewMenu->addAction(
                tr("Path Effects", "MenuBar_View"));
    mPathEffectsVisible->setCheckable(true);
    mPathEffectsVisible->setChecked(true);
    connect(mPathEffectsVisible, &QAction::triggered,
            &mActions, &Actions::setPathEffectsVisible);


    mPanelsMenu = mViewMenu->addMenu(tr("Docks", "MenuBar_View"));

    mSelectedObjectDockAct = mPanelsMenu->addAction(
                tr("Selected Objects", "MenuBar_View_Docks"));
    mSelectedObjectDockAct->setCheckable(true);
    mSelectedObjectDockAct->setChecked(true);
    mSelectedObjectDockAct->setShortcut(QKeySequence(Qt::Key_O));

    connect(mSelectedObjectDock, &CloseSignalingDockWidget::madeVisible,
            mSelectedObjectDockAct, &QAction::setChecked);
    connect(mSelectedObjectDockAct, &QAction::toggled,
            mSelectedObjectDock, &QDockWidget::setVisible);

    mFilesDockAct = mPanelsMenu->addAction(
                tr("Files", "MenuBar_View_Docks"));
    mFilesDockAct->setCheckable(true);
    mFilesDockAct->setChecked(true);
    mFilesDockAct->setShortcut(QKeySequence(Qt::Key_F));

    connect(mFilesDock, &CloseSignalingDockWidget::madeVisible,
            mFilesDockAct, &QAction::setChecked);
    connect(mFilesDockAct, &QAction::toggled,
            mFilesDock, &QDockWidget::setVisible);

    mTimelineDockAct = mPanelsMenu->addAction(
                tr("Timeline", "MenuBar_View_Docks"));
    mTimelineDockAct->setCheckable(true);
    mTimelineDockAct->setChecked(true);
    mTimelineDockAct->setShortcut(QKeySequence(Qt::Key_T));

    connect(mTimelineDock, &CloseSignalingDockWidget::madeVisible,
            mTimelineDockAct, &QAction::setChecked);
    connect(mTimelineDockAct, &QAction::toggled,
            mTimelineDock, &QDockWidget::setVisible);

    mFillAndStrokeDockAct = mPanelsMenu->addAction(
                tr("Fill and Stroke", "MenuBar_View_Docks"));
    mFillAndStrokeDockAct->setCheckable(true);
    mFillAndStrokeDockAct->setChecked(true);
    mFillAndStrokeDockAct->setShortcut(QKeySequence(Qt::Key_E));

    connect(mFillStrokeSettingsDock, &CloseSignalingDockWidget::madeVisible,
            mFillAndStrokeDockAct, &QAction::setChecked);
    connect(mFillAndStrokeDockAct, &QAction::toggled,
            mFillStrokeSettingsDock, &QDockWidget::setVisible);

    mBrushDockAction = mPanelsMenu->addAction(
                tr("Paint Brush", "MenuBar_View_Docks"));
    mBrushDockAction->setCheckable(true);
    mBrushDockAction->setChecked(false);
    mBrushDockAction->setShortcut(QKeySequence(Qt::Key_B));

    connect(mBrushSettingsDock, &CloseSignalingDockWidget::madeVisible,
            mBrushDockAction, &QAction::setChecked);
    connect(mBrushDockAction, &QAction::toggled,
            mBrushSettingsDock, &QDockWidget::setVisible);

    mAlignDockAction = mPanelsMenu->addAction(
                tr("Align", "MenuBar_View_Docks"));
    mAlignDockAction->setCheckable(true);
    mAlignDockAction->setChecked(false);
    mAlignDockAction->setShortcut(QKeySequence(Qt::Key_D));

    connect(mAlignDock, &CloseSignalingDockWidget::madeVisible,
            mAlignDockAction, &QAction::setChecked);
    connect(mAlignDockAction, &QAction::toggled,
            mAlignDock, &QDockWidget::setVisible);

    mBrushColorBookmarksAction = mPanelsMenu->addAction(
                tr("Brush/Color Bookmarks", "MenuBar_View_Docks"));
    mBrushColorBookmarksAction->setCheckable(true);
    mBrushColorBookmarksAction->setChecked(true);
    mBrushColorBookmarksAction->setShortcut(QKeySequence(Qt::Key_U));

    connect(mBrushColorBookmarksAction, &QAction::toggled,
            mCentralWidget, &CentralWidget::setSidesVisibilitySetting);

    const auto help = mMenuBar->addMenu(tr("Help", "MenuBar"));

    help->addAction(tr("License", "MenuBar_Help"), this, [this]() {
        if(EnveLicense::sInstance) {
            delete EnveLicense::sInstance;
        } else {
            const auto license = new EnveLicense(this);
            license->show();
        }
    });

    mMenuBar->addSeparator();
    mMenuBar->addAction(tr("Support enve", "MenuBar"), this, []() {
        QDesktopServices::openUrl(QUrl("https://maurycyliebner.github.io/"));
    });

    setMenuBar(mMenuBar);
    mMenuBar->setStyleSheet("QMenuBar { padding-top: 1px; }");
}


#include "welcomedialog.h"
void MainWindow::openWelcomeDialog() {
    if(mWelcomeDialog) return;
    mWelcomeDialog = new WelcomeDialog(getRecentFiles(),
       [this]() { SceneSettingsDialog::sNewSceneDialog(mDocument, this); },
       []() { MainWindow::sGetInstance()->openFile(); },
       [](QString path) { MainWindow::sGetInstance()->openFile(path); },
       this);
    takeCentralWidget();
    setCentralWidget(mWelcomeDialog);
}

void MainWindow::closeWelcomeDialog() {
    SimpleTask::sScheduleContexted(this, [this]() {
        if(!mWelcomeDialog) return;
        mWelcomeDialog = nullptr;
        setCentralWidget(mCentralWidget);
    });
}

void MainWindow::addCanvasToRenderQue() {
    if(!mDocument.fActiveScene) return;
    mTimeline->getRenderWidget()->
    createNewRenderInstanceWidgetForCanvas(mDocument.fActiveScene);
}

void MainWindow::updateSettingsForCurrentCanvas(Canvas* const scene) {
    mObjectSettingsWidget->setCurrentScene(scene);
    if(!scene) {
        mObjectSettingsWidget->setMainTarget(nullptr);
        mTimeline->updateSettingsForCurrentCanvas(nullptr);
        return;
    }
    mClipViewToCanvas->setChecked(scene->clipToCanvas());
    mRasterEffectsVisible->setChecked(scene->getRasterEffectsVisible());
    mPathEffectsVisible->setChecked(scene->getPathEffectsVisible());
    mTimeline->updateSettingsForCurrentCanvas(scene);
    mObjectSettingsWidget->setMainTarget(scene->getCurrentGroup());
}

#include <QSpacerItem>
void MainWindow::setupStatusBar() {
    mUsageWidget = new UsageWidget(this);
    setStatusBar(mUsageWidget);
}

void MainWindow::setupToolBar() {
    mToolBar = new QToolBar(tr("Toolbar"), this);
    mToolBar->setMovable(false);

    eSizesUI::button.add(mToolBar, [this](const int size) {
        mToolBar->setIconSize(QSize(size, size));
    });

    mToolBar->addSeparator();

    mBoxTransformMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/boxTransformUnchecked.png",
                "toolbarButtons/boxTransformChecked.png",
                gSingleLineTooltip(tr("Object Mode", "ToolBar"), "F1"), this);
    mBoxTransformMode->toggle();
    mToolBar->addWidget(mBoxTransformMode);

    mPointTransformMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/pointTransformUnchecked.png",
                "toolbarButtons/pointTransformChecked.png",
                gSingleLineTooltip(tr("Point Mode", "ToolBar"), "F2"), this);
    mToolBar->addWidget(mPointTransformMode);

    mAddPointMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/pathCreateUnchecked.png",
                "toolbarButtons/pathCreateChecked.png",
                gSingleLineTooltip(tr("Add Path Mode", "ToolBar"), "F3"), this);
    mToolBar->addWidget(mAddPointMode);


    mDrawPathMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/drawPathUnchecked.png",
                "toolbarButtons/drawPathChecked.png",
                gSingleLineTooltip(tr("Draw Path Mode", "ToolBar"), "F4"), this);
    mToolBar->addWidget(mDrawPathMode);

    mToolBar->addSeparator();

    mPaintMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/paintUnchecked.png",
                "toolbarButtons/paintChecked.png",
                gSingleLineTooltip(tr("Paint Mode", "ToolBar"), "F5"), this);
    mToolBar->addWidget(mPaintMode);

    mCircleMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/circleCreateUnchecked.png",
                "toolbarButtons/circleCreateChecked.png",
                gSingleLineTooltip(tr("Add Circle Mode", "ToolBar"), "F6"), this);
    mToolBar->addWidget(mCircleMode);

    mRectangleMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/rectCreateUnchecked.png",
                "toolbarButtons/rectCreateChecked.png",
                gSingleLineTooltip(tr("Add Rectange Mode", "ToolBar"), "F7"), this);
    mToolBar->addWidget(mRectangleMode);

    mTextMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/textCreateUnchecked.png",
                "toolbarButtons/textCreateChecked.png",
                gSingleLineTooltip(tr("Add Text Mode", "ToolBar"), "F8"), this);
    mToolBar->addWidget(mTextMode);

    mToolBar->addSeparator();

    mNullMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/nullCreateUnchecked.png",
                "toolbarButtons/nullCreateChecked.png",
                gSingleLineTooltip(tr("Add Null Object Mode", "ToolBar"), "F9"), this);
    mToolBar->addWidget(mNullMode);

    mPickPaintSettingsMode = SwitchButton::sCreate2Switch(
                "toolbarButtons/pickUnchecked.png",
                "toolbarButtons/pickChecked.png",
                gSingleLineTooltip(tr("Pick Mode", "ToolBar"), "F10"), this);
    mToolBar->addWidget(mPickPaintSettingsMode);

    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("emptyToolButton");
    mToolBar->addWidget(mLayoutHandler->comboWidget());
    mToolBar->widgetForAction(mToolBar->addAction("     "))->
            setObjectName("emptyToolButton");

    mActionConnectPoints = new ActionButton("toolbarButtons/nodeConnect.png",
                                            tr("Connect Nodes", "ToolBar"), this);
    mActionConnectPointsAct = mToolBar->addWidget(mActionConnectPoints);

    mActionDisconnectPoints = new ActionButton("toolbarButtons/nodeDisconnect.png",
                                               tr("Disconnect Nodes", "ToolBar"), this);
    mActionDisconnectPointsAct = mToolBar->addWidget(mActionDisconnectPoints);

    mActionMergePoints = new ActionButton("toolbarButtons/nodeMerge.png",
                                          tr("Merge Nodes", "ToolBar"), this);
    mActionMergePointsAct = mToolBar->addWidget(mActionMergePoints);

    mActionNewNode = new ActionButton("toolbarButtons/nodeNew.png",
                                      tr("New Node", "ToolBar"), this);
    mActionNewNodeAct = mToolBar->addWidget(mActionNewNode);
//
    mSeparator1 = mToolBar->addSeparator();

    mActionSymmetricPointCtrls = new ActionButton("toolbarButtons/nodeSymmetric.png",
                                                  tr("Symmetric Nodes", "ToolBar"), this);
    mActionSymmetricPointCtrlsAct = mToolBar->addWidget(mActionSymmetricPointCtrls);

    mActionSmoothPointCtrls = new ActionButton("toolbarButtons/nodeSmooth.png",
                                               tr("Smooth Nodes", "ToolBar"), this);
    mActionSmoothPointCtrlsAct = mToolBar->addWidget(mActionSmoothPointCtrls);

    mActionCornerPointCtrls = new ActionButton("toolbarButtons/nodeCorner.png",
                                               tr("Corner Nodes", "ToolBar"), this);
    mActionCornerPointCtrlsAct = mToolBar->addWidget(mActionCornerPointCtrls);

//
    mSeparator2 = mToolBar->addSeparator();

    mActionLine = new ActionButton("toolbarButtons/segmentLine.png",
                                   gSingleLineTooltip(tr("Make Segment Line", "ToolBar")), this);
    mActionLineAct = mToolBar->addWidget(mActionLine);

    mActionCurve = new ActionButton("toolbarButtons/segmentCurve.png",
                                    gSingleLineTooltip(tr("Make Segment Curve", "ToolBar")), this);
    mActionCurveAct = mToolBar->addWidget(mActionCurve);

    mFontWidget = new FontsWidget(this);
    mFontWidgetAct = mToolBar->addWidget(mFontWidget);

    mActionNewEmptyPaintFrame = new ActionButton(
                "toolbarButtons/newEmpty.png",
                gSingleLineTooltip(tr("New Empty Frame", "ToolBar"), "N"), this);
    mActionNewEmptyPaintFrameAct = mToolBar->addWidget(mActionNewEmptyPaintFrame);

    eSizesUI::widget.add(mToolBar, [this](const int size) {
        mToolBar->setFixedHeight(2*size);
    });

    addToolBar(mToolBar);

    mDocument.setCanvasMode(CanvasMode::boxTransform);
}

void MainWindow::connectToolBarActions() {
    connect(mBoxTransformMode, &ActionButton::pressed,
            &mActions, &Actions::setMovePathMode);
    connect(mPointTransformMode, &ActionButton::pressed,
            &mActions, &Actions::setMovePointMode);
    connect(mAddPointMode, &ActionButton::pressed,
            &mActions, &Actions::setAddPointMode);
    connect(mDrawPathMode, &ActionButton::pressed,
            &mActions, &Actions::setDrawPathMode);

    connect(mPaintMode, &ActionButton::pressed,
            &mActions, &Actions::setPaintMode);
    connect(mCircleMode, &ActionButton::pressed,
            &mActions, &Actions::setCircleMode);
    connect(mRectangleMode, &ActionButton::pressed,
            &mActions, &Actions::setRectangleMode);
    connect(mTextMode, &ActionButton::pressed,
            &mActions, &Actions::setTextMode);

    connect(mNullMode, &ActionButton::pressed,
            &mActions, &Actions::setNullMode);
    connect(mPickPaintSettingsMode, &ActionButton::pressed,
            &mActions, &Actions::setPickPaintSettingsMode);

    connect(mActionConnectPoints, &ActionButton::pressed,
            &mActions, &Actions::connectPointsSlot);
    connect(mActionDisconnectPoints, &ActionButton::pressed,
            &mActions, &Actions::disconnectPointsSlot);
    connect(mActionMergePoints, &ActionButton::pressed,
            &mActions, &Actions::mergePointsSlot);
    connect(mActionNewNode, &ActionButton::pressed,
            &mActions, &Actions::subdivideSegments);

    connect(mActionSymmetricPointCtrls, &ActionButton::pressed,
            &mActions, &Actions::makePointCtrlsSymmetric);
    connect(mActionSmoothPointCtrls, &ActionButton::pressed,
            &mActions, &Actions::makePointCtrlsSmooth);
    connect(mActionCornerPointCtrls, &ActionButton::pressed,
            &mActions, &Actions::makePointCtrlsCorner);

    connect(mActionLine, &ActionButton::pressed,
            &mActions, &Actions::makeSegmentLine);
    connect(mActionCurve, &ActionButton::pressed,
            &mActions, &Actions::makeSegmentCurve);

    connect(mFontWidget, &FontsWidget::fontSizeChanged,
            &mActions, &Actions::setFontSize);
    connect(mFontWidget, &FontsWidget::fontFamilyAndStyleChanged,
            &mActions, &Actions::setFontFamilyAndStyle);
    connect(mFontWidget, &FontsWidget::textAlignmentChanged,
            &mActions, &Actions::setTextAlignment);
    connect(mFontWidget, &FontsWidget::textVAlignmentChanged,
            &mActions, &Actions::setTextVAlignment);

    connect(mActionNewEmptyPaintFrame, &ActionButton::pressed,
            &mActions, &Actions::newEmptyPaintFrame);
}

MainWindow *MainWindow::sGetInstance() {
    return sInstance;
}

void MainWindow::updateCanvasModeButtonsChecked() {
    const CanvasMode mode = mDocument.fCanvasMode;
    mCentralWidget->setCanvasMode(mode);

    mBoxTransformMode->setState(mode == CanvasMode::boxTransform);
    mPointTransformMode->setState(mode == CanvasMode::pointTransform);
    mAddPointMode->setState(mode == CanvasMode::pathCreate);
    mDrawPathMode->setState(mode == CanvasMode::drawPath);

    mPaintMode->setState(mode == CanvasMode::paint);
    mCircleMode->setState(mode == CanvasMode::circleCreate);
    mRectangleMode->setState(mode == CanvasMode::rectCreate);
    mTextMode->setState(mode == CanvasMode::textCreate);

    mNullMode->setState(mode == CanvasMode::nullCreate);
    mPickPaintSettingsMode->setState(mode == CanvasMode::pickFillStroke);

    const bool boxMode = mode == CanvasMode::boxTransform;
    mFontWidgetAct->setVisible(boxMode);

    const bool pointMode = mode == CanvasMode::pointTransform;
    mActionConnectPointsAct->setVisible(pointMode);
    mActionDisconnectPointsAct->setVisible(pointMode);
    mActionMergePointsAct->setVisible(pointMode);
    mActionNewNodeAct->setVisible(pointMode);
    mActionSymmetricPointCtrlsAct->setVisible(pointMode);
    mActionSmoothPointCtrlsAct->setVisible(pointMode);
    mActionCornerPointCtrlsAct->setVisible(pointMode);
    mActionLineAct->setVisible(pointMode);
    mActionCurveAct->setVisible(pointMode);
    mSeparator1->setVisible(pointMode);
    mSeparator2->setVisible(pointMode);

    const bool paintMode = mode == CanvasMode::paint;
    mActionNewEmptyPaintFrameAct->setVisible(paintMode);
    if(paintMode) {
        mFillStrokeSettingsDock->setWidget(mPaintColorWidget);
    } else {
        mFillStrokeSettingsDock->setWidget(mFillStrokeSettings);
    }
}

//void MainWindow::stopPreview() {
//    mPreviewInterrupted = true;
//    if(!mRendering) {
//        mCurrentRenderFrame = mMaxFrame;
//        mCanvas->clearPreview();
//        mCanvasWindow->repaint();
//        previewFinished();
//    }
//}

void MainWindow::setResolutionValue(const qreal value) {
    if(!mDocument.fActiveScene) return;
    mDocument.fActiveScene->setResolution(value);
    mDocument.actionFinished();
}

void MainWindow::setFileChangedSinceSaving(const bool changed) {
    if(changed == mChangedSinceSaving) return;
    mChangedSinceSaving = changed;
    updateTitle();
}

SimpleBrushWrapper *MainWindow::getCurrentBrush() const {
    return mBrushSelectionWidget->getCurrentBrush();
}

#include "Boxes/textbox.h"
void MainWindow::setCurrentBox(BoundingBox *box) {
    mFillStrokeSettings->setCurrentBox(box);
    if(const auto txtBox = enve_cast<TextBox*>(box)) {
        mFontWidget->setDisplayedSettings(txtBox->getFontSize(),
                                          txtBox->getFontFamily(),
                                          txtBox->getFontStyle());
    }
}

FillStrokeSettingsWidget *MainWindow::getFillStrokeSettings() {
    return mFillStrokeSettings;
}

bool MainWindow::askForSaving() {
    if(mChangedSinceSaving) {
        const QString title = tr("Save", "AskSaveDialog_Title");
        const QString fileName = mDocument.fEvFile.split("/").last();

        const QString question = tr("Save changes to document \"%1\"?",
                                    "AskSaveDialog_Question");
        const QString questionWithTarget = question.arg(fileName);
        const QString closeNoSave =  tr("Close without saving",
                                        "AskSaveDialog_Button");
        const QString cancel = tr("Cancel", "AskSaveDialog_Button");
        const QString save = tr("Save", "AskSaveDialog_Button");
        const int buttonId = QMessageBox::question(
                    this, title, questionWithTarget,
                    closeNoSave, cancel, save);
        if(buttonId == 1) {
            return false;
        } else if(buttonId == 2) {
            saveFile();
            return true;
        }
    }
    return true;
}

BoxScrollWidget *MainWindow::getObjectSettingsList() {
    return mObjectSettingsWidget;
}

void MainWindow::disableEventFilter() {
    mEventFilterDisabled = true;
}

void MainWindow::enableEventFilter() {
    mEventFilterDisabled = false;
}

void MainWindow::disable() {
    disableEventFilter();
    mGrayOutWidget = new QWidget(this);
    mGrayOutWidget->setFixedSize(size());
    mGrayOutWidget->setStyleSheet(
                "QWidget { background-color: rgb(0, 0, 0, 125) }");
    mGrayOutWidget->show();
    mGrayOutWidget->update();
}

void MainWindow::enable() {
    if(!mGrayOutWidget) return;
    enableEventFilter();
    delete mGrayOutWidget;
    mGrayOutWidget = nullptr;
    mDocument.actionFinished();
}

void MainWindow::newFile() {
    if(askForSaving()) {
        closeProject();
        SceneSettingsDialog::sNewSceneDialog(mDocument, this);
    }
}

bool handleCanvasModeKeyPress(Document& document, const int key) {
    if(key == Qt::Key_F1) {
        document.setCanvasMode(CanvasMode::boxTransform);
    } else if(key == Qt::Key_F2) {
        document.setCanvasMode(CanvasMode::pointTransform);
    } else if(key == Qt::Key_F3) {
        document.setCanvasMode(CanvasMode::pathCreate);
    }  else if(key == Qt::Key_F4) {
        document.setCanvasMode(CanvasMode::drawPath);
    } else if(key == Qt::Key_F5) {
        document.setCanvasMode(CanvasMode::paint);
    } else if(key == Qt::Key_F6) {
        document.setCanvasMode(CanvasMode::circleCreate);
    } else if(key == Qt::Key_F7) {
        document.setCanvasMode(CanvasMode::rectCreate);
    } else if(key == Qt::Key_F8) {
        document.setCanvasMode(CanvasMode::textCreate);
    } else if(key == Qt::Key_F9) {
        document.setCanvasMode(CanvasMode::nullCreate);
    } else if(key == Qt::Key_F10) {
        document.setCanvasMode(CanvasMode::pickFillStroke);
    } else return false;
    KeyFocusTarget::KFT_sSetRandomTarget();
    return true;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e) {
    if(mLock) if(dynamic_cast<QInputEvent*>(e)) return true;
    if(mEventFilterDisabled) return QMainWindow::eventFilter(obj, e);
    const auto type = e->type();
    const auto focusWidget = QApplication::focusWidget();
    if(type == QEvent::KeyPress) {
        const auto keyEvent = static_cast<QKeyEvent*>(e);
        if(keyEvent->key() == Qt::Key_Delete && focusWidget) {
            mEventFilterDisabled = true;
            const bool widHandled =
                    QCoreApplication::sendEvent(focusWidget, keyEvent);
            mEventFilterDisabled = false;
            if(widHandled) return false;
        }
        return processKeyEvent(keyEvent);
    } else if(type == QEvent::ShortcutOverride) {
        const auto keyEvent = static_cast<QKeyEvent*>(e);
        const int key = keyEvent->key();
        if(key == Qt::Key_Tab) {
            KeyFocusTarget::KFT_sTab();
            return true;
        }
        if(handleCanvasModeKeyPress(mDocument, key)) return true;
        if(keyEvent->modifiers() & Qt::SHIFT && key == Qt::Key_D) {
            return processKeyEvent(keyEvent);
        }
        if(keyEvent->modifiers() & Qt::CTRL) {
            if(key == Qt::Key_C || key == Qt::Key_V ||
               key == Qt::Key_X || key == Qt::Key_D) {
                return processKeyEvent(keyEvent);
            }
        } else if(key == Qt::Key_A || key == Qt::Key_I ||
                  key == Qt::Key_Delete) {
              return processKeyEvent(keyEvent);
        }
    } else if(type == QEvent::KeyRelease) {
        const auto keyEvent = static_cast<QKeyEvent*>(e);
        if(processKeyEvent(keyEvent)) return true;
        //finishUndoRedoSet();
    } else if(type == QEvent::MouseButtonRelease) {
        //finishUndoRedoSet();
    }
    return QMainWindow::eventFilter(obj, e);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    if(!closeProject()) e->ignore();
}

bool MainWindow::processKeyEvent(QKeyEvent *event) {
    if(isActiveWindow()) {
        bool returnBool = false;
        if(event->type() == QEvent::KeyPress &&
            mTimeline->processKeyPress(event)) {
            returnBool = true;
        } else {
            returnBool = KeyFocusTarget::KFT_handleKeyEvent(event);
        }
        mDocument.actionFinished();
        return returnBool;
    }
    return false;
}

bool MainWindow::isEnabled() {
    return !mGrayOutWidget;
}

void MainWindow::clearAll() {
    TaskScheduler::instance()->clearTasks();
    setFileChangedSinceSaving(false);
    mObjectSettingsWidget->setMainTarget(nullptr);

    mTimeline->clearAll();
    mFillStrokeSettings->clearAll();
    mDocument.clear();
    mLayoutHandler->clear();
//    for(ClipboardContainer *cont : mClipboardContainers) {
//        delete cont;
//    }
//    mClipboardContainers.clear();
    FilesHandler::sInstance->clear();
    //mBoxListWidget->clearAll();
    openWelcomeDialog();
}

void MainWindow::updateTitle() {
    QString star = "";
    if(mChangedSinceSaving) star = "*";
    setWindowTitle(mDocument.fEvFile.split("/").last() + star + " - enve");
}

void MainWindow::openFile() {
    if(askForSaving()) {
        disable();
        const QString defPath = mDocument.fEvFile.isEmpty() ?
                    QDir::homePath() : mDocument.fEvFile;

        const QString title = tr("Open File", "OpenDialog_Title");
        const QString files = tr("enve Files %1", "OpenDialog_FileType");
        const QString openPath = eDialogs::openFile(title, defPath,
                                                    files.arg("(*.ev *.xev)"));
        if(!openPath.isEmpty()) openFile(openPath);
        enable();
    }
}

void MainWindow::openFile(const QString& openPath) {
    clearAll();
    try {
        QFileInfo fi(openPath);
        const QString suffix = fi.suffix();
        if(suffix == "ev") {
            loadEVFile(openPath);
        } else if(suffix == "xev") {
            loadXevFile(openPath);
        } else RuntimeThrow("Unrecognized file extension " + suffix);
        mDocument.setPath(openPath);
        setFileChangedSinceSaving(false);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
    mDocument.actionFinished();
}

void MainWindow::saveFile() {
    if(mDocument.fEvFile.isEmpty()) {
        saveFileAs(true);
    } else saveFile(mDocument.fEvFile);
}

void MainWindow::saveFile(const QString& path, const bool setPath) {
    try {
        QFileInfo fi(path);
        const QString suffix = fi.suffix();
        if(suffix == "ev") {
            saveToFile(path);
        } else if(suffix == "xev") {
            saveToFileXEV(path);
            const auto& inst = DialogsInterface::instance();
            inst.displayMessageToUser("Please note that the XEV format is still in the testing phase.");
        } else RuntimeThrow("Unrecognized file extension " + suffix);
        if(setPath) mDocument.setPath(path);
        setFileChangedSinceSaving(false);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

void MainWindow::saveFileAs(const bool setPath) {
    disableEventFilter();
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;

    const QString title = tr("Save File", "SaveDialog_Title");
    const QString fileType = tr("enve Files %1", "SaveDialog_FileType");
    QString saveAs = eDialogs::saveFile(title, defPath, fileType.arg("(*.ev *.xev)"));
    enableEventFilter();
    if(!saveAs.isEmpty()) {
        const bool isXEV = saveAs.right(4) == ".xev";
        if(!isXEV && saveAs.right(3) != ".ev") saveAs += ".ev";

        saveFile(saveAs, setPath);
    }
}

void MainWindow::saveBackup() {
    const QString backupPath = "backup/backup_%1.ev";
    int id = 1;
    QFile backupFile(backupPath.arg(id));
    while(backupFile.exists()) {
        id++;
        backupFile.setFileName(backupPath.arg(id) );
    }
    try {
        saveToFile(backupPath.arg(id));
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
}

void MainWindow::exportSVG() {
    const auto dialog = new ExportSvgDialog(this);
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
}

bool MainWindow::closeProject() {
    if(askForSaving()) {
        clearAll();
        return true;
    }
    return false;
}

void MainWindow::importFile() {
    disableEventFilter();
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;

    const QString title = tr("Import File(s)", "ImportDialog_Title");
    const QString fileType = tr("Files %1", "ImportDialog_FileTypes");
    const QString fileTypes = "(*.ev *.xev *.svg " +
            FileExtensions::videoFilters() +
            FileExtensions::imageFilters() +
            FileExtensions::soundFilters() + ")";
    const auto importPaths = eDialogs::openFiles(
                title, defPath, fileType.arg(fileTypes));
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        for(const QString &path : importPaths) {
            if(path.isEmpty()) continue;
            try {
                mActions.importFile(path);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
    }
}

void MainWindow::linkFile() {
    disableEventFilter();
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;
    const QString title = tr("Link File", "LinkDialog_Title");
    const QString fileType = tr("Files %1", "LinkDialog_FileType");
    const auto importPaths = eDialogs::openFiles(
                title, defPath, fileType.arg("(*.svg *.ora *.kra)"));
    enableEventFilter();
    if(!importPaths.isEmpty()) {
        for(const QString &path : importPaths) {
            if(path.isEmpty()) continue;
            try {
                mActions.linkFile(path);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
    }
}

void MainWindow::importImageSequence() {
    disableEventFilter();
    const QString defPath = mDocument.fEvFile.isEmpty() ?
                QDir::homePath() : mDocument.fEvFile;
    const QString title = tr("Import Image Sequence",
                             "ImportSequenceDialog_Title");
    const auto folder = eDialogs::openDir(title, defPath);
    enableEventFilter();
    if(!folder.isEmpty()) mActions.importFile(folder);
}

void MainWindow::revert() {
    const QString path = mDocument.fEvFile;
    openFile(path);
}

stdsptr<void> MainWindow::lock() {
    if(mLock) return mLock->ref<Lock>();
    setEnabled(false);
    const auto newLock = enve::make_shared<Lock>(this);
    mLock = newLock.get();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    return newLock;
}

void MainWindow::lockFinished() {
    if(mLock) {
        gPrintException(false, "Lock finished before lock object deleted");
    } else {
        QApplication::restoreOverrideCursor();
        setEnabled(true);
    }
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    if(statusBar()) statusBar()->setMaximumWidth(width());
    QMainWindow::resizeEvent(e);
}

void MainWindow::showEvent(QShowEvent *e) {
    if(statusBar()) statusBar()->setMaximumWidth(width());
    QMainWindow::showEvent(e);
}

void MainWindow::updateRecentMenu() {
    mRecentMenu->clear();
    const auto homePath = QDir::homePath();
    for(const auto& path : mRecentFiles) {
        QString ttPath = path;
        if(ttPath.left(homePath.count()) == homePath) {
            ttPath = "~" + ttPath.mid(homePath.count());
        }
        mRecentMenu->addAction(path, [path, this]() {
            openFile(path);
        });
    }
}
