#ifndef UNICOMM_NUSPELLMODULE_H
#define UNICOMM_NUSPELLMODULE_H

#include <QWidget>
#include <nuspell/dictionary.hxx>

class NuspellModule final : public QWidget {
    Q_OBJECT

public:
    explicit NuspellModule(QWidget *parent = nullptr);

    ~NuspellModule() override = default;

private:
    nuspell::Dictionary m_dict{};
};

#endif //UNICOMM_NUSPELLMODULE_H