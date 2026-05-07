#ifndef UNICOMM_WELCOMEPAGE_H
#define UNICOMM_WELCOMEPAGE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;

class WelcomePage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit WelcomePage();

    ~WelcomePage() override = default;

    void propertySet(const QVariantHash &objects);

    Q_INVOKABLE void workspaceOpen();

signals:
    void openWorkspace();

private:
    QQuickWidget *m_welcomeWidget{};
};

#endif //UNICOMM_WELCOMEPAGE_H
