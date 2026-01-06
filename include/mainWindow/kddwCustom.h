#ifndef UNICOMM_KDDWCUSTOM_H
#define UNICOMM_KDDWCUSTOM_H

#include <kddockwidgets/qtwidgets/Separator.h>
#include <kddockwidgets/qtwidgets/ViewFactory.h>

class MySeparator final : public KDDockWidgets::QtWidgets::Separator {
public:
    explicit MySeparator(KDDockWidgets::Core::Separator *controller, KDDockWidgets::Core::View *parent);

    ~MySeparator() override = default;
};

class CustomWidgetFactory final : public KDDockWidgets::QtWidgets::ViewFactory {
    Q_OBJECT

public:
    KDDockWidgets::Core::View *createSeparator(KDDockWidgets::Core::Separator *, KDDockWidgets::Core::View *parent = nullptr) const override;
};

#endif //UNICOMM_KDDWCUSTOM_H
