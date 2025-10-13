#ifndef UNICOMM_WELCOMEPAGE_H
#define UNICOMM_WELCOMEPAGE_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class WelcomePage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit WelcomePage();

    ~WelcomePage() override = default;

signals:
    void openWorkspace();
};

#endif //UNICOMM_WELCOMEPAGE_H
