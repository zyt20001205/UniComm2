#ifndef UNICOMM_NUSPELLMODULE_H
#define UNICOMM_NUSPELLMODULE_H

#include <QWidget>
#include <nuspell/dictionary.hxx>

class NuspellModule final : public QWidget {
    Q_OBJECT

public:
    explicit NuspellModule(QWidget *parent = nullptr);

    ~NuspellModule() override = default;

    void spellCheckFileRequest(const QUrl &scriptUrl);

    void spellCheckWordRequest(const QString &word) const;

private:
    QStringList spellCheck(const QString &word) const;

    nuspell::Dictionary m_dict{};

    enum {
        PLAIN,
        UPPERCAMEL,
        LOWERCAMEL
    };
};

#endif //UNICOMM_NUSPELLMODULE_H
