#ifndef DEBUG_H
#define DEBUG_H

#include <QComboBox>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "luaInterpreter.h"

class Debug final : public QDockWidget {
    Q_OBJECT

public:
    explicit Debug(QWidget *parent = nullptr);

    ~Debug() override = default;

    void debugStart(const QString &threadId, LuaInterpreter *interpreter);

signals:
    void resume(const QString &threadId);

private:
    QHash<QString, LuaInterpreter *> m_interpreterHash;
    QComboBox *m_debugThreadCombobox{};

    enum {
        STATE_RUN,
        STATE_PAUSE,
        STATE_TERMINATE,
        STATE_STEPOVER,
        STATE_STEPINTO,
        STATE_STEPOUT,
    };
};

#endif //DEBUG_H
