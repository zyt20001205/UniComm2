#ifndef UNICOMM_UNDOMODULE_H
#define UNICOMM_UNDOMODULE_H

#include <functional>
#include <type_traits>
#include <utility>

#include <QHash>
#include <QSharedPointer>
#include <QUndoStack>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class UndoGroupData;

class UndoModule final : public QUndoStack {
    Q_OBJECT

public:
    explicit UndoModule(QObject *parent = nullptr);

    ~UndoModule() override = default;

    template<typename Redo, typename Undo>
    QString push(const QString &text, Redo redo, Undo undo, const QString &undoGroupId = {}) {
        auto redoAction = _action(std::move(redo));
        auto undoAction = _action(std::move(undo));
        if (undoGroupId.isEmpty()) return _push(text, std::move(redoAction), std::move(undoAction), std::function<void(const QString &)>{});
        return _push(text, std::move(redoAction), std::move(undoAction), undoGroupId);
    }

    template<typename Redo, typename Undo>
    QString push(const QString &text, Redo redo, Undo undo, std::function<void(const QString &)> failed) {
        return _push(text, _action(std::move(redo)), _action(std::move(undo)), std::move(failed));
    }

    [[nodiscard]] QString undoGroupBegin();

    [[nodiscard]] QString undoGroupCommit(const QString &undoGroupId, const QString &text, std::function<void(const QString &)> failed = {});

    [[nodiscard]] QString undoGroupRevert(const QString &undoGroupId);

    void undoGroupRelease(const QString &undoGroupId);

    void undoGroupInvalidate(const QString &undoGroupId, const QString &error);

    [[nodiscard]] QVariantMap undoGroupGet(const QString &undoGroupId) const;

    Q_INVOKABLE [[nodiscard]] QVariantList undoHistory() const;

    Q_INVOKABLE [[nodiscard]] QVariantList redoHistory() const;

    Q_INVOKABLE void undoTo(int targetIndex);

    Q_INVOKABLE void redoTo(int targetIndex);

signals:
    void updateChange(const QString &undoGroupId);

private:
    template<typename Action>
    static std::function<QString()> _action(Action action) {
        using Result = std::invoke_result_t<Action &>;
        static_assert(std::is_void_v<Result> || std::is_convertible_v<Result, QString>);

        if constexpr (std::is_void_v<Result>) {
            return [action = std::move(action)]() mutable -> QString {
                std::invoke(action);
                return QString{};
            };
        } else {
            return [action = std::move(action)]() mutable -> QString {
                return std::invoke(action);
            };
        }
    }

    QString _push(const QString &text, std::function<QString()> redo, std::function<QString()> undo, std::function<void(const QString &)> failed);

    QString _push(const QString &text, std::function<QString()> redo, std::function<QString()> undo, const QString &undoGroupId);

    QHash<QString, QSharedPointer<UndoGroupData>> m_undoGroups{};
};

class UndoGroupData final {
public:
    struct Step {
        QString text{};
        std::function<QString()> redo{};
        std::function<QString()> undo{};
    };

    [[nodiscard]] QString redo();

    [[nodiscard]] QString undo();

    QString text{};
    QString error{};
    QVector<Step> steps{};
    std::function<void(const QString &)> failed{};
    bool committed{};
    bool applied{true};

private:
    [[nodiscard]] QString execute(bool redo);
};

class UndoCommand final : public QUndoCommand {
public:
    UndoCommand(const QString &text, std::function<QString()> redo, std::function<QString()> undo, QSharedPointer<QString> result);

    void redo() override;

    void undo() override;

    void failedSet(std::function<void(const QString &)> failed);

private:
    void execute(const std::function<QString()> &action);

    std::function<QString()> m_redo;
    std::function<QString()> m_undo;
    std::function<void(const QString &)> m_failed;
    QSharedPointer<QString> m_result;
};

#endif //UNICOMM_UNDOMODULE_H
