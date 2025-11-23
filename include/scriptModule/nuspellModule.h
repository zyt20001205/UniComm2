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

signals:
    void responseSpellCheck(const QUrl &scriptUrl, const QVariantList &misspellings);

private:
    bool spellCheck(const QString &word) const;

    QVariantList spellSuggest(const QString &word) const;

    nuspell::Dictionary m_dict{};
};

#endif //UNICOMM_NUSPELLMODULE_H
