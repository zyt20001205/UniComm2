#ifndef UNICOMM_PIXMAPPREVIEW_H
#define UNICOMM_PIXMAPPREVIEW_H

#include <QDialog>

class QVBoxLayout;

class PixmapPreview final : public QDialog {
    Q_OBJECT

public:
    explicit PixmapPreview(QWidget *parent = nullptr);

    ~PixmapPreview() override = default;

    void previewShow(const QList<QPixmap> &pixmapList) const;

private:
    void previewClear() const;

    QVBoxLayout *m_layout{};
};

#endif //UNICOMM_PIXMAPPREVIEW_H
