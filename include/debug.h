#ifndef DEBUG_H
#define DEBUG_H

#include <QComboBox>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include "threadpool.h"

extern Threadpool *g_threadpool;

class Debug final : public QDockWidget {
    Q_OBJECT

public:
    explicit Debug(QWidget *parent = nullptr);

    ~Debug() override = default;

    void debugStart();

signals:
    void resumeDebug();

private:
    void debugThreadGet() const;

    QComboBox *m_debugThreadCombobox{};
};

#endif //DEBUG_H
