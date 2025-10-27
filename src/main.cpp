#include "mainWindow.h"

#include <QStyleFactory>
#include <kddockwidgets/Config.h>

#include <visa.h>

void test() {
    ViSession rm = VI_NULL;
    ViSession instr = VI_NULL;
    ViFindList findList;
    ViUInt32 numInst;
    ViChar instrDesc[VI_FIND_BUFLEN];
    ViStatus status;
    ViUInt32 retCount;

    status = viOpenDefaultRM(&rm);
    status = viFindRsrc(rm, "?*INSTR", &findList, &numInst, instrDesc);
    status = viOpen(rm, instrDesc, VI_NULL, VI_NULL, &instr);
    status = viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);
    ViChar command[] = "OUTP1:STAT OFF\r\n";
    status = viWrite(instr, (ViBuf) command, (ViUInt32) strlen(command), &retCount);
    ViChar response[256] = {0};
    status = viRead(instr, (ViBuf) response, sizeof(response) - 1, &retCount);
    qDebug() << response;

    // qDebug() << instrDesc;
    // for (ViUInt32 i = 1; i < numInst; i++) {
    //     status = viFindNext(findList, instrDesc);
    //     qDebug() << instrDesc;
    // }

    viClose(findList);
    viClose(rm);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtWidgets);
    auto flags = KDDockWidgets::Config::self().flags();
    flags |= KDDockWidgets::Config::Flag_TabsHaveCloseButton;
    flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
    flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
    KDDockWidgets::Config::self().setFlags(flags);
    KDDockWidgets::InitialOption::s_defaultNeighbourSqueezeStrategy = KDDockWidgets::NeighbourSqueezeStrategy::AllNeighbours;

    auto *mainWindow = new MainWindow();
    mainWindow->show();

    return QApplication::exec();
}
