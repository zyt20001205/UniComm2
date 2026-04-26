#ifndef UNICOMM_DATAPLOT_H
#define UNICOMM_DATAPLOT_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class QQuickWidget;

class DataplotModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DataplotModule();

    ~DataplotModule() override;

    void propertySet(const QVariantMap &objects);

signals:

private:
    QQuickWidget *m_widget{};
};

#endif //UNICOMM_DATAPLOT_H
