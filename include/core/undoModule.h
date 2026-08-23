#ifndef UNICOMM_UNDOMODULE_H
#define UNICOMM_UNDOMODULE_H

#include <functional>

#include <QUndoStack>

class UndoModule final : public QUndoStack {
    Q_OBJECT

public:
    explicit UndoModule(QObject *parent = nullptr);

    ~UndoModule() override = default;

    using QUndoStack::push;

    void push(const QString &text, std::function<void()> redo, std::function<void()> undo);
};

#endif //UNICOMM_UNDOMODULE_H
