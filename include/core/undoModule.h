#ifndef UNICOMM_UNDOMODULE_H
#define UNICOMM_UNDOMODULE_H

#include <functional>

#include <QUndoStack>
#include <QVariantList>

class UndoModule final : public QUndoStack {
    Q_OBJECT

public:
    explicit UndoModule(QObject *parent = nullptr);

    ~UndoModule() override = default;

    using QUndoStack::push;

    void push(const QString &text, std::function<void()> redo, std::function<void()> undo);

    Q_INVOKABLE [[nodiscard]] QVariantList undoHistory() const;

    Q_INVOKABLE [[nodiscard]] QVariantList redoHistory() const;

    Q_INVOKABLE void undoTo(int targetIndex);

    Q_INVOKABLE void redoTo(int targetIndex);
};

#endif //UNICOMM_UNDOMODULE_H
