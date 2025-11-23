#ifndef UNICOMM_NUSPELLMODULE_H
#define UNICOMM_NUSPELLMODULE_H

#include <QWidget>
#include <nuspell/dictionary.hxx>

class NuspellModule final : public QWidget {
    Q_OBJECT

public:
    explicit NuspellModule(QWidget *parent = nullptr);

    ~NuspellModule() override = default;

    void spellCheckRequest(const QUrl &scriptUrl, const QString &script);

    void spellSuggestRequest(const QUrl &scriptUrl, const QString &word);

signals:
    void responseSpellCheck(const QUrl &scriptUrl, const QVariantList &typos);

    void responseSpellSuggest(const QUrl &scriptUrl, const QString &word, const QStringList &suggestions);

private:
    nuspell::Dictionary m_dict{};
};

#endif //UNICOMM_NUSPELLMODULE_H
