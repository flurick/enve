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

#include "GUI/mainwindow.h"
#include <iostream>
#include <QApplication>
#include <QSurfaceFormat>
#include <QProcess>
#include "hardwareinfo.h"
#include "Private/esettings.h"
#include "GUI/ewidgetsimpl.h"
#include "importhandler.h"
#include "effectsloader.h"
#include "memoryhandler.h"
#include "ShaderEffects/shadereffectprogram.h"
#include "videoencoder.h"
#include "iconloader.h"
#include "GUI/envesplash.h"
#ifdef Q_OS_WIN
    #include "windowsincludes.h"
#endif // Q_OS_WIN
extern "C" {
    #include <libavformat/avformat.h>
}

#include <QJSEngine>

#define TIME_BEGIN const auto t1 = std::chrono::high_resolution_clock::now();
#define TIME_END(name) const auto t2 = std::chrono::high_resolution_clock::now(); \
                       const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count(); \
                       qDebug() << name << duration << "us" << endl;

#define GPU_NOT_COMPATIBLE gPrintException("Your GPU drivers do not seem to be compatible.")

void printHardware() {
    std::cout << "Hardware:" << std::endl;
    std::cout << "    CPU Threads: " << HardwareInfo::sCpuThreads() << std::endl;
    std::cout << "         Memory: " << intMB(HardwareInfo::sRamKB()).fValue << "MB" << std::endl;
    std::cout << "            GPU: ";
    switch(HardwareInfo::sGpuVendor()) {
    case GpuVendor::amd: std::cout << "AMD"; break;
    case GpuVendor::intel: std::cout << "Intel"; break;
    case GpuVendor::nvidia: std::cout << "Nvidia"; break;
    case GpuVendor::unrecognized: std::cout << "Unrecognized"; break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

void setDefaultFormat() {
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(0);
    //format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    //format.setSwapInterval(0); // Disable vertical refresh syncing
    QSurfaceFormat::setDefaultFormat(format);
}

void generateAlphaMesh(QPixmap& alphaMesh, const int dim) {
    alphaMesh = QPixmap(2*dim, 2*dim);
    const QColor light = QColor::fromRgbF(0.2, 0.2, 0.2);
    const QColor dark = QColor::fromRgbF(0.4, 0.4, 0.4);
    QPainter p(&alphaMesh);
    p.fillRect(0, 0, dim, dim, light);
    p.fillRect(dim, 0, dim, dim, dark);
    p.fillRect(0, dim, dim, dim, dark);
    p.fillRect(dim, dim, dim, dim, light);
    p.end();
}

int main(int argc, char *argv[]) {
    std::cout << "Entered main()" << std::endl;
#ifdef Q_OS_WIN
    SetProcessDPIAware(); // call before the main event loop
#endif // Q_OS_WIN

#ifdef Q_OS_LINUX
    // qputenv("QT_QPA_PLATFORMTHEME", "gtk2");
#endif

    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    setDefaultFormat();
    std::cout << "Setup Default QSurfaceFormat" << std::endl;
    QApplication app(argc, argv);
    app.setStyleSheet("QStatusBar::item { border: 0; }");
    setlocale(LC_NUMERIC, "C");

    const bool threadedOpenGL = QOpenGLContext::supportsThreadedOpenGL();
    if(!threadedOpenGL) {
        gPrintException("Your GPU drivers do not support OpenGL "
                        "rendering outside the main thread");
    }
    try {
        HardwareInfo::sUpdateInfo();
    } catch(const std::exception& e) {
        GPU_NOT_COMPATIBLE;
        gPrintExceptionCritical(e);
    }
    printHardware();

    eSettings settings(HardwareInfo::sCpuThreads(),
                       HardwareInfo::sRamKB(),
                       HardwareInfo::sGpuVendor());

    OS_FONT = QApplication::font();
    eSizesUI::font.setEvaluator([&settings]() {
        const auto fm = QFontMetrics(OS_FONT);
        const qreal scaling = qBound(0.5, settings.fInterfaceScaling, 1.5);
        return qRound(fm.height()*scaling);
    });
    eSizesUI::widget.setEvaluator([]() {
        return eSizesUI::font.size()*4/3;
    });
    QObject::connect(&eSizesUI::font, &SizeSetter::sizeChanged,
                     &eSizesUI::widget, &SizeSetter::updateSize);
    eSizesUI::font.add(&app, [&app](const int size) {
        const auto fm = QFontMetrics(OS_FONT);
        const qreal mult = size/qreal(fm.height());
        QFont font = OS_FONT;
        if(OS_FONT.pixelSize() == -1) {
            font.setPointSizeF(mult*OS_FONT.pointSizeF());
        } else {
            font.setPixelSize(qRound(mult*OS_FONT.pixelSize()));
        }
        app.setFont(font);
    });

    eSizesUI::widget.add(&eSizesUI::button, [](const int size) {
        eSizesUI::button.set(qRound(size*1.1));
    });

    eSizesUI::widget.add([](const int size) {
        KEY_RECT_SIZE = size*3/5;
    });

    QPixmap alphaMesh;
    eSizesUI::widget.add([&alphaMesh](const int size) {
        generateAlphaMesh(alphaMesh, size/2);
    });
    ALPHA_MESH_PIX = &alphaMesh;
    std::cout << "Generated Alpha Mesh" << std::endl;

    //#ifdef QT_DEBUG
    //    const qint64 pId = QCoreApplication::applicationPid();
    //    QProcess * const process = new QProcess(&w);
    //    process->start("prlimit --data=3000000000 --pid " + QString::number(pId));
    //#endif

    try {
        settings.loadFromFile();
        std::cout << "Loaded settings" << std::endl;
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }

    eFilterSettings filterSettings;
    QDir(eSettings::sSettingsDir()).mkpath(eSettings::sIconsDir());
    try {
        const QString pngPath = eSettings::sIconsDir() + "/splash1.png";
#ifdef QT_DEBUG
        QFile(pngPath).remove();
#endif
        IconLoader::generate(":/pixmaps/splash.svg", eSizesUI::widget/22., pngPath);
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
    const auto splash = new EnveSplash;
    splash->show();
    app.processEvents();

    splash->showMessage("Generate icons...");
    app.processEvents();
    eSizesUI::button.add([](const int size) {
        IconLoader::generateAll(eSizesUI::widget, size);
    });

    eWidgetsImpl widImpl;
    ImportHandler importHandler;

    splash->showMessage("Initialize task scheduler...");
    app.processEvents();
    MemoryHandler memoryHandler;
    TaskScheduler taskScheduler;
    QObject::connect(&memoryHandler, &MemoryHandler::enteredCriticalState,
                     &taskScheduler, &TaskScheduler::enterCriticalMemoryState);
    QObject::connect(&memoryHandler, &MemoryHandler::finishedCriticalState,
                     &taskScheduler, &TaskScheduler::finishCriticalMemoryState);

    Document document(taskScheduler);
    Actions actions(document);

    splash->showMessage("Initialize gpu resources...");
    app.processEvents();
    EffectsLoader effectsLoader;
    try {
        effectsLoader.initializeGpu();
        taskScheduler.initializeGpu();
    } catch(const std::exception& e) {
        GPU_NOT_COMPATIBLE;
        gPrintExceptionFatal(e);
    }

    splash->showMessage("Initialize custom path effects...");
    app.processEvents();
    effectsLoader.iniCustomPathEffects();
    std::cout << "Custom path effects initialized" << std::endl;

    splash->showMessage("Initialize custom raster effects...");
    app.processEvents();
    effectsLoader.iniCustomRasterEffects();
    std::cout << "Custom raster effects initialized" << std::endl;

    splash->showMessage("Initialize shader effects...");
    app.processEvents();
    try {
        effectsLoader.iniShaderEffects();
    } catch(const std::exception& e) {
        GPU_NOT_COMPATIBLE;
        gPrintExceptionCritical(e);
    }
    QObject::connect(&effectsLoader, &EffectsLoader::programChanged,
    [&document](ShaderEffectProgram * program) {
        for(const auto& scene : document.fScenes)
            scene->updateIfUsesProgram(program);
        document.actionFinished();
    });
    std::cout << "Shader effects initialized" << std::endl;

    splash->showMessage("Initialize custom objects...");
    app.processEvents();
    effectsLoader.iniCustomBoxes();
    std::cout << "Custom objects initialized" << std::endl;

    splash->showMessage("Initialize audio...");
    app.processEvents();

    eSoundSettings soundSettings;
    AudioHandler audioHandler;

    try {
        audioHandler.initializeAudio(soundSettings.sData());
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
    }
    std::cout << "Audio initialized" << std::endl;

    splash->showMessage("Initialize render handler...");
    app.processEvents();
    const auto videoEncoder = enve::make_shared<VideoEncoder>();
    RenderHandler renderHandler(document, audioHandler,
                                *videoEncoder, memoryHandler);
    std::cout << "Render handler initialized" << std::endl;

    MainWindow w(document, actions, audioHandler, renderHandler);
    if(argc > 1) {
        try {
            splash->showMessage("Load file...");
            app.processEvents();
            w.openFile(argv[1]);
        } catch(const std::exception& e) {
            gPrintExceptionCritical(e);
        }
    }
    splash->showMessage("Done");
    app.processEvents();
    w.show();

    const bool keepSplashVisible = true;
    if(keepSplashVisible) {
        splash->setParent(&w);
        splash->move(splash->pos() + w.mapFromGlobal({0, 0}));
        splash->show();
    } else splash->finish(&w);

    try {
        return app.exec();
    } catch(const std::exception& e) {
        gPrintExceptionFatal(e);
        return -1;
    }
}
