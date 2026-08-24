#include "core/undoModule.h"

#include <utility>

// public
UndoModule::UndoModule(QObject *parent)
    : QUndoStack(parent) {
    this->setUndoLimit(100);
}

QVariantList UndoModule::undoHistory() const {
    QVariantList history;
    const int currentIndex = index();
    history.reserve(currentIndex);
    for (int commandIndex = currentIndex - 1; commandIndex >= 0; --commandIndex) {
        history.append(QVariantMap{
            {"text", text(commandIndex)},
            {"targetIndex", commandIndex},
            {"steps", currentIndex - commandIndex}
        });
    }
    return history;
}

QVariantList UndoModule::redoHistory() const {
    QVariantList history;
    const int currentIndex = index();
    history.reserve(count() - currentIndex);
    for (int commandIndex = currentIndex; commandIndex < count(); ++commandIndex) {
        history.append(QVariantMap{
            {"text", text(commandIndex)},
            {"targetIndex", commandIndex + 1},
            {"steps", commandIndex - currentIndex + 1}
        });
    }
    return history;
}

void UndoModule::undoTo(const int targetIndex) {
    if (targetIndex < 0 || targetIndex >= index()) return;
    setIndex(targetIndex);
}

void UndoModule::redoTo(const int targetIndex) {
    if (targetIndex <= index() || targetIndex > count()) return;
    setIndex(targetIndex);
}

// private
QString UndoModule::_push(const QString &text, std::function<QString()> redo, std::function<QString()> undo, std::function<void(const QString &)> failed) {
    const auto result = QSharedPointer<QString>::create();
    auto *command = new UndoCommand(text, std::move(redo), std::move(undo), result);
    QUndoStack::push(command);
    // A failed initial redo marks the command obsolete, so QUndoStack has already deleted it.
    if (result->isEmpty()) command->failedSet(std::move(failed));
    return *result;
}

// public
UndoCommand::UndoCommand(const QString &text, std::function<QString()> redo, std::function<QString()> undo, QSharedPointer<QString> result)
    : QUndoCommand(text),
      m_redo(std::move(redo)),
      m_undo(std::move(undo)),
      m_result(std::move(result)) {
}

void UndoCommand::redo() {
    execute(m_redo);
}

void UndoCommand::undo() {
    execute(m_undo);
}

void UndoCommand::failedSet(std::function<void(const QString &)> failed) {
    m_failed = std::move(failed);
}

// private
void UndoCommand::execute(const std::function<QString()> &action) {
    *m_result = action();
    if (m_result->isEmpty()) return;

    setObsolete(true);
    if (m_failed) m_failed(*m_result);
}
