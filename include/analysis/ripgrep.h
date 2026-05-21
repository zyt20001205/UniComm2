#ifndef UNICOMM_RIPGREP_H
#define UNICOMM_RIPGREP_H

#include <QObject>

class QProcess;

class Ripgrep final : public QObject {
    Q_OBJECT

public:
    explicit Ripgrep(QWidget *parent = nullptr);

    ~Ripgrep() override = default;

    [[nodiscard]] static QJsonArray grep(const QString &pattern);

private:
};

#endif //UNICOMM_RIPGREP_H
