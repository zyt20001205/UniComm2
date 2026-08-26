#include "core/undoModule.h"

#include <QUuid>

#include <utility>

// public
UndoModule::UndoModule(QObject *parent)
    : QUndoStack(parent) {
    this->setUndoLimit(100);
}

QString UndoModule::undoGroupBegin() {
    QString undoGroupId{};
    do undoGroupId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    while (m_undoGroups.contains(undoGroupId));
    m_undoGroups.insert(undoGroupId, QSharedPointer<UndoGroupData>::create());
    return undoGroupId;
}

QString UndoModule::undoGroupCommit(const QString &undoGroupId, const QString &text, std::function<void(const QString &)> failed) {
    const auto undoGroup = m_undoGroups.value(undoGroupId);
    if (undoGroup.isNull()) return tr("Undo group commit failed: group does not exist.");
    if (undoGroup->committed) return tr("Undo group commit failed: group is already committed.");

    undoGroup->committed = true;
    undoGroup->text = text;
    undoGroup->failed = std::move(failed);
    if (undoGroup->steps.isEmpty() && undoGroup->error.isEmpty()) {
        m_undoGroups.remove(undoGroupId);
        return {};
    }

    const auto initial = QSharedPointer<bool>::create(true);
    return _push(
        text,
        [undoGroup, initial] {
            if (*initial) {
                *initial = false;
                return QString{};
            }
            return undoGroup->redo();
        },
        [undoGroup] { return undoGroup->undo(); },
        undoGroup->failed);
}

QString UndoModule::undoGroupRevert(const QString &undoGroupId) {
    const auto undoGroup = m_undoGroups.value(undoGroupId);
    if (undoGroup.isNull()) return tr("Undo group revert failed: group does not exist.");
    if (!undoGroup->committed) return tr("Undo group revert failed: group is not committed.");
    if (!undoGroup->error.isEmpty()) return undoGroup->error;
    if (!undoGroup->applied) return tr("Undo group revert failed: group is already reverted.");

    return _push(
        tr("Revert %1").arg(undoGroup->text),
        [undoGroup] { return undoGroup->undo(); },
        [undoGroup] { return undoGroup->redo(); },
        undoGroup->failed);
}

void UndoModule::undoGroupRelease(const QString &undoGroupId) {
    m_undoGroups.remove(undoGroupId);
}

void UndoModule::undoGroupInvalidate(const QString &undoGroupId, const QString &error) {
    if (error.isEmpty()) return;
    const auto undoGroup = m_undoGroups.value(undoGroupId);
    if (!undoGroup.isNull() && undoGroup->error.isEmpty()) undoGroup->error = error;
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

QString UndoModule::_push(const QString &text, std::function<QString()> redo, std::function<QString()> undo, const QString &undoGroupId) {
    const auto undoGroup = m_undoGroups.value(undoGroupId);
    if (undoGroup.isNull()) return tr("Undo group push failed: group does not exist.");
    if (undoGroup->committed) return tr("Undo group push failed: group is already committed.");

    const auto error = redo();
    if (!error.isEmpty()) return error;
    undoGroup->steps.append({text, std::move(redo), std::move(undo)});
    return {};
}

// public
QString UndoGroupData::redo() {
    if (!error.isEmpty()) return error;
    if (applied) return QObject::tr("Undo group redo failed: group is already applied.");
    return execute(true);
}

QString UndoGroupData::undo() {
    if (!error.isEmpty()) return error;
    if (!applied) return QObject::tr("Undo group undo failed: group is already reverted.");
    return execute(false);
}

// private
QString UndoGroupData::execute(const bool redo) {
    if (!error.isEmpty()) return error;

    QVector<int> completed{};
    completed.reserve(steps.size());
    for (int offset = 0; offset < steps.size(); ++offset) {
        const int index = redo ? offset : steps.size() - offset - 1;
        const auto actionError = redo ? steps.at(index).redo() : steps.at(index).undo();
        if (actionError.isEmpty()) {
            completed.append(index);
            continue;
        }

        QString compensationError{};
        for (auto completedIndex = completed.crbegin(); completedIndex != completed.crend(); ++completedIndex) {
            const auto rollbackError = redo ? steps.at(*completedIndex).undo() : steps.at(*completedIndex).redo();
            if (!rollbackError.isEmpty() && compensationError.isEmpty()) compensationError = rollbackError;
        }
        if (compensationError.isEmpty()) return actionError;
        error = QObject::tr("%1 Compensation failed: %2").arg(actionError, compensationError);
        return error;
    }

    applied = redo;
    return {};
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
