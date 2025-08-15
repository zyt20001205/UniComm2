#ifndef INIT_H
#define INIT_H

#include <QDebug>
#include <QMainWindow>
#include "config.h"
#include "log.h"
#include "port.h"
#include "send.h"
#include "script.h"

class Init {
public:
    static void configInit();

    static void moduleInit(QMainWindow* mainWindow);
};

#endif //INIT_H
