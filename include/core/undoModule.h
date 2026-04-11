#ifndef UNICOMM_UNDOMODULE_H
#define UNICOMM_UNDOMODULE_H

#include <QUndoStack>

class QMainWindow;

class UndoModule final : public QUndoStack {
    Q_OBJECT

public:
    explicit UndoModule(QObject  *parent = nullptr);

    ~UndoModule() override = default;;
};

#endif //UNICOMM_UNDOMODULE_H
