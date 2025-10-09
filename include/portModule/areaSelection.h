#ifndef AREASELECTION_H
#define AREASELECTION_H

#include <QDialog>

class QComboBox;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QListView;
class QRadioButton;
class QSlider;
class QStackedWidget;
class QStandardItemModel;

class AreaSelect final : public QDialog {
    Q_OBJECT

public:
    explicit AreaSelect(QWidget *parent = nullptr);

    ~AreaSelect() override = default;

    void reload(const QJsonObject &config);

    void captureRequest(const QString &type, const QString &target);

    double dprExport() const;

    QString charsetExport() const;

    QJsonObject processExport() const;

    QJsonArray areaExport() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void processRequest();

    void selectionRequest() const;

    void ocrRequest() const;

    void charsetRequest();

    QString m_type{};
    QString m_target{};
    QGraphicsView *m_graphicsView{};
    QGraphicsScene *m_graphicsScene{};
    double m_dpr{};

    QComboBox *m_processComboBox{};
    QStackedWidget *m_processStackedWidget{};
    int m_processType = 0;

    QSlider *m_gaussianblurSlider{};
    QLabel *m_gaussianblurValueLabel{};
    int m_kernalSize = 1;

    QWidget *m_thresholdValueWidget{};
    QSlider *m_thresholdSlider{};
    QLabel *m_thresholdValueLabel{};
    int m_thresholdValue = 128;
    QComboBox *m_thresholdTypeComboBox{};
    QRadioButton *m_thresholdManual{};
    QRadioButton *m_thresholdOtsu{};
    QRadioButton *m_thresholdTriangle{};
    int m_thresholdType = 1;

    QString m_charsetString = "eng";
    QListView *m_charsetListView{};
    QStandardItemModel *m_charsetModel{};
    QListView *m_selectionListView{};
    QStandardItemModel *m_selectionModel{};

    QPixmap m_shot{};
    QPixmap m_pshot{};
    QRectF m_rectF{};
};

#endif //AREASELECTION_H
