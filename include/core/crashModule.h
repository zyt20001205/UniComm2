#ifndef UNICOMM_CRASHMODULE_H
#define UNICOMM_CRASHMODULE_H

#include <QStringList>

class CrashModule final {
public:
    CrashModule();

    [[nodiscard]] static bool reporterMode(const QStringList &arguments);

    static int reporterExec(const QStringList &arguments);

private:
    static constexpr auto ReporterArgument = "--crash-reporter";
    static constexpr int DarkTheme = 1;
};

#endif //UNICOMM_CRASHMODULE_H
