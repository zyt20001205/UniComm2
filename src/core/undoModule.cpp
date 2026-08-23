#include "core/undoModule.h"

#include <utility>

#include <QUndoCommand>

namespace {
class FunctionUndoCommand final : public QUndoCommand {
public:
    FunctionUndoCommand(const QString &text, std::function<void()> redo, std::function<void()> undo)
        : QUndoCommand(text),
          m_redo(std::move(redo)),
          m_undo(std::move(undo)) {
    }

    void redo() override {
        m_redo();
    }

    void undo() override {
        m_undo();
    }

private:
    std::function<void()> m_redo;
    std::function<void()> m_undo;
};
}

UndoModule::UndoModule(QObject *parent)
    : QUndoStack(parent) {
    this->setUndoLimit(100);
}

void UndoModule::push(const QString &text, std::function<void()> redo, std::function<void()> undo) {
    QUndoStack::push(new FunctionUndoCommand(text, std::move(redo), std::move(undo)));
}
