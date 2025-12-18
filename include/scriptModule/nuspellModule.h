#ifndef UNICOMM_NUSPELLMODULE_H
#define UNICOMM_NUSPELLMODULE_H

#include <QObject>
#include <nuspell/dictionary.hxx>

class NuspellModule final : public QObject {
    Q_OBJECT

public:
    explicit NuspellModule(QObject *parent = nullptr);

    ~NuspellModule() override = default;

    void spellCheckRequest(const QUrl &scriptUrl, const QString &script);

    QStringList spellSuggestRequest(const QString &word) const;

signals:
    void responseSpellCheck(const QUrl &scriptUrl, const QVariantList &typos);

private:
    nuspell::Dictionary m_dict{};
};

#endif //UNICOMM_NUSPELLMODULE_H
