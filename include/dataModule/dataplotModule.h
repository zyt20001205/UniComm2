#ifndef UNICOMM_DATAPLOT_H
#define UNICOMM_DATAPLOT_H

#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class DataplotModule final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit DataplotModule();

    ~DataplotModule() override;

signals:

private:
};

#endif //UNICOMM_DATAPLOT_H
