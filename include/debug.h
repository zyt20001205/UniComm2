#ifndef DEBUG_H
#define DEBUG_H

#include <QComboBox>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <qtableview.h>
#include <QVBoxLayout>

class LuaInterpreter;

class Debug final : public QDockWidget {
    Q_OBJECT

public:
    explicit Debug(QWidget *parent = nullptr);

    ~Debug() override = default;

    void breakpointInsert(const QUrl &scriptUrl, int line);

    void breakpointRemove(const QUrl &scriptUrl, int line);

    void debugStart(const QString &threadId, LuaInterpreter *interpreter);

    void debugEnd(const QString &threadId);

signals:
    void resume(const QString &threadId);

private:
    QHash<QString, LuaInterpreter *> m_interpreterHash{};
    QComboBox *m_debugThreadCombobox{};
    QTableView *m_debugThreadTableView{};

    enum {
        DEBUG_RUN,
        DEBUG_PAUSE,
        DEBUG_STEPOVER,
        DEBUG_STEPINTO,
        DEBUG_STEPOUT
    };
};

#endif //DEBUG_H
