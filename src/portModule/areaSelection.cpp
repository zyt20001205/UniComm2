#include "portModule/areaSelection.h"

#include <QButtonGroup>
#include <QCamera>
#include <QComboBox>
#include <qgraphicsitem.h>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QImageCapture>
#include <QJsonArray>
#include <QKeyEvent>
#include <QLabel>
#include <QListView>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPushButton>
#include <QRadioButton>
#include <QScreen>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardItem>

#include "globals.h"
#include "utils.h"

// AreaSelection public
AreaSelection::AreaSelection(QWidget *parent)
    : QDialog(parent) {
    this->setFixedSize(1280, 720);
    auto *layout = new QHBoxLayout(this); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    auto *splitter = new QSplitter(Qt::Horizontal); // NOLINT
    layout->addWidget(splitter);

    m_graphicsView = new QGraphicsView();
    splitter->addWidget(m_graphicsView);
    connect(m_graphicsView, &QGraphicsView::rubberBandChanged, this, [this](const QRectF &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint) {
        const bool rubberBandEnded = viewportRect.isNull();
        auto sceneRect = QRectF(fromScenePoint, toScenePoint);
        if (!sceneRect.isNull() && sceneRect.isValid()) {
            m_rectF = sceneRect;
        }
        if (rubberBandEnded) {
            const auto logicalRect = m_rectF;
            const auto physicalRect = QRectF(m_rectF.x() * m_dpr, m_rectF.y() * m_dpr, m_rectF.width() * m_dpr, m_rectF.height() * m_dpr);
            auto *item = new QStandardItem(); // NOLINT
            const QPixmap cropped = m_shot.copy(physicalRect.toRect());
            const QString recognizedText = ocr(cropped, m_charsetString);
            item->setText(recognizedText);
            item->setData(QVariant::fromValue(logicalRect), Qt::UserRole + 1);
            item->setData(QVariant::fromValue(physicalRect), Qt::UserRole + 2);
            item->setFlags(item->flags() &= ~Qt::ItemIsDropEnabled);
            m_selectionModel->appendRow(item);
            selectionRequest();
        }
    });
    m_graphicsScene = new QGraphicsScene();
    m_graphicsView->setScene(m_graphicsScene);
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    auto *ctrlWidget = new QWidget(); // NOLINT
    splitter->addWidget(ctrlWidget);
    auto *ctrlLayout = new QVBoxLayout(ctrlWidget); // NOLINT
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setAlignment(Qt::AlignLeft);
    auto *refreshButton = new QPushButton("refresh"); // NOLINT
    ctrlLayout->addWidget(refreshButton);
    refreshButton->setFixedSize(80, 48);
    refreshButton->setIcon(QIcon(":/icon/arrowClockwise.svg"));
    connect(refreshButton, &QPushButton::clicked, this, [this] { captureRequest(m_type, m_target); });

    auto *selectButton = new QPushButton("select"); // NOLINT
    ctrlLayout->addWidget(selectButton);
    selectButton->setCheckable(true);
    selectButton->setFixedSize(80, 48);
    selectButton->setIcon(QIcon(":/icon/crop.svg"));
    connect(selectButton, &QPushButton::clicked, this, [this](const bool status) {
        if (status) {
            m_graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
        } else {
            m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
        }
    });

    // charset widget
    {
        auto *charsetWidget = new QWidget(); // NOLINT
        ctrlLayout->addWidget(charsetWidget);
        auto *charsetLayout = new QVBoxLayout(charsetWidget); // NOLINT
        charsetLayout->setContentsMargins(0, 0, 0, 0);

        auto *charsetLabel = new QLabel(tr("Charset")); // NOLINT
        charsetLayout->addWidget(charsetLabel);
        charsetLabel->setFont(QFont("consolas", 16));

        auto *seperator = new QFrame(); // NOLINT
        seperator->setFrameShape(QFrame::HLine);
        seperator->setLineWidth(1);
        charsetLayout->addWidget(seperator);

        m_charsetListView = new QListView();
        charsetLayout->addWidget(m_charsetListView);
        m_charsetListView->setStyleSheet("QListView::item { min-height: 30px; }");
        m_charsetListView->setDragDropMode(QAbstractItemView::InternalMove);
        m_charsetListView->setDefaultDropAction(Qt::MoveAction);
        m_charsetListView->setDragEnabled(true);
        m_charsetListView->setAcceptDrops(true);
        m_charsetListView->setDropIndicatorShown(true);
        m_charsetModel = new QStandardItemModel();
        m_charsetListView->setModel(m_charsetModel);
        auto *engItem = new QStandardItem(); // NOLINT
        m_charsetModel->appendRow(engItem);
        engItem->setText(tr("English"));
        engItem->setData("eng", Qt::UserRole + 1);
        engItem->setCheckable(true);
        engItem->setCheckState(Qt::Checked);
        engItem->setFlags(engItem->flags() &= ~Qt::ItemIsDropEnabled);
        auto *ssegItem = new QStandardItem(); // NOLINT
        m_charsetModel->appendRow(ssegItem);
        ssegItem->setText(tr("Seven Segment"));
        ssegItem->setData("7seg", Qt::UserRole + 1);
        ssegItem->setCheckable(true);
        ssegItem->setFlags(ssegItem->flags() &= ~Qt::ItemIsDropEnabled);
        connect(m_charsetModel, &QStandardItemModel::rowsRemoved, this, [this] { charsetRequest(); });
        connect(m_charsetModel, &QStandardItemModel::itemChanged, this, [this] { charsetRequest(); });
    }

    // process widget
    {
        auto *processWidget = new QWidget(); // NOLINT
        ctrlLayout->addWidget(processWidget);
        auto *processLayout = new QVBoxLayout(processWidget); // NOLINT
        processLayout->setContentsMargins(0, 0, 0, 0);

        auto *processLabel = new QLabel(tr("Process")); // NOLINT
        processLayout->addWidget(processLabel);
        processLabel->setFont(QFont("consolas", 16));

        auto *seperator = new QFrame(); // NOLINT
        seperator->setFrameShape(QFrame::HLine);
        seperator->setLineWidth(1);
        processLayout->addWidget(seperator);

        m_processStackedWidget = new QStackedWidget();
        m_processComboBox = new QComboBox();
        ctrlLayout->addWidget(m_processComboBox);
        m_processComboBox->addItem(tr("Raw"));
        m_processComboBox->addItem(tr("Gaussian Blur"));
        m_processComboBox->addItem(tr("Threshold"));
        connect(m_processComboBox, &QComboBox::activated, this, [this] {
            m_processType = m_processComboBox->currentIndex();
            m_processStackedWidget->setCurrentIndex(m_processType);
            processRequest();
        });

        ctrlLayout->addWidget(m_processStackedWidget);
        // raw
        {
            auto *rawWidget = new QWidget(); // NOLINT
            m_processStackedWidget->addWidget(rawWidget);
        }
        // gaussianblur
        {
            auto *gaussianblurWidget = new QWidget(); // NOLINT
            m_processStackedWidget->addWidget(gaussianblurWidget);
            auto *gaussianblurLayout = new QHBoxLayout(gaussianblurWidget); // NOLINT
            gaussianblurLayout->setContentsMargins(0, 0, 0, 0);
            auto *gaussianblurLabel = new QLabel(tr("Kernal Size")); // NOLINT
            gaussianblurLayout->addWidget(gaussianblurLabel);
            m_gaussianblurSlider = new QSlider(Qt::Horizontal);
            gaussianblurLayout->addWidget(m_gaussianblurSlider);
            m_gaussianblurSlider->setRange(1, 20);
            m_gaussianblurSlider->setValue(m_kernalSize);
            connect(m_gaussianblurSlider, &QSlider::valueChanged, [this] {
                m_kernalSize = m_gaussianblurSlider->value();
                m_gaussianblurValueLabel->setText(QString::number(m_kernalSize));
            });
            connect(m_gaussianblurSlider, &QSlider::sliderReleased, [this] {
                processRequest();
            });
            m_gaussianblurValueLabel = new QLabel(QString::number(m_kernalSize));
            gaussianblurLayout->addWidget(m_gaussianblurValueLabel);
        }
        // threshold
        {
            auto *thresholdWidget = new QWidget(); // NOLINT
            m_processStackedWidget->addWidget(thresholdWidget);
            auto *thresholdLayout = new QVBoxLayout(thresholdWidget); // NOLINT
            thresholdLayout->setContentsMargins(0, 0, 0, 0);

            m_thresholdValueWidget = new QWidget();
            thresholdLayout->addWidget(m_thresholdValueWidget);
            auto *thresholdValueLayout = new QHBoxLayout(m_thresholdValueWidget); // NOLINT
            thresholdValueLayout->setContentsMargins(0, 0, 0, 0);
            auto *label = new QLabel(tr("Thresh")); // NOLINT
            thresholdValueLayout->addWidget(label);
            m_thresholdSlider = new QSlider(Qt::Horizontal);
            thresholdValueLayout->addWidget(m_thresholdSlider);
            m_thresholdSlider->setRange(0, 255);
            m_thresholdSlider->setValue(m_thresholdValue);
            connect(m_thresholdSlider, &QSlider::valueChanged, [this] {
                m_thresholdValue = m_thresholdSlider->value();
                m_thresholdValueLabel->setText(QString::number(m_thresholdValue));
            });
            connect(m_thresholdSlider, &QSlider::sliderReleased, [this] {
                processRequest();
            });
            m_thresholdValueLabel = new QLabel(QString::number(m_thresholdValue));
            thresholdValueLayout->addWidget(m_thresholdValueLabel);

            auto *thresholdTypeWidget = new QWidget(); // NOLINT
            thresholdLayout->addWidget(thresholdTypeWidget);
            auto *thresholdTypeLayout = new QVBoxLayout(thresholdTypeWidget); // NOLINT
            thresholdTypeLayout->setContentsMargins(0, 0, 0, 0);
            m_thresholdTypeComboBox = new QComboBox();
            thresholdTypeLayout->addWidget(m_thresholdTypeComboBox);
            m_thresholdTypeComboBox->addItem(tr("Binary"));
            m_thresholdTypeComboBox->addItem(tr("Binary Inverted"));
            m_thresholdTypeComboBox->addItem(tr("Truncate"));
            m_thresholdTypeComboBox->addItem(tr("To Zero"));
            m_thresholdTypeComboBox->addItem(tr("To Zero Inverted"));
            m_thresholdTypeComboBox->setCurrentIndex(1);
            connect(m_thresholdTypeComboBox, &QComboBox::activated, this, [this](const int index) {
                m_thresholdType &= 0b11000;
                m_thresholdType |= index;
                processRequest();
            });
            auto *buttonGroup = new QButtonGroup(); // NOLINT
            m_thresholdManual = new QRadioButton(tr("Manual"));
            thresholdTypeLayout->addWidget(m_thresholdManual);
            buttonGroup->addButton(m_thresholdManual);
            m_thresholdManual->setChecked(true);
            connect(m_thresholdManual, &QRadioButton::toggled, [this] {
                m_thresholdType &= 0b00111;
                m_thresholdValueWidget->show();
                processRequest();
            });
            m_thresholdOtsu = new QRadioButton(tr("Otsu"));
            thresholdTypeLayout->addWidget(m_thresholdOtsu);
            buttonGroup->addButton(m_thresholdOtsu);
            connect(m_thresholdOtsu, &QRadioButton::toggled, [this] {
                m_thresholdType &= 0b00111;
                m_thresholdType |= 0b01000;
                m_thresholdValueWidget->hide();
                processRequest();
            });
            m_thresholdTriangle = new QRadioButton(tr("Triangle"));
            thresholdTypeLayout->addWidget(m_thresholdTriangle);
            buttonGroup->addButton(m_thresholdTriangle);
            m_thresholdManual->setChecked(true);
            connect(m_thresholdTriangle, &QRadioButton::toggled, [this] {
                m_thresholdType &= 0b00111;
                m_thresholdType |= 0b10000;
                m_thresholdValueWidget->hide();
                processRequest();
            });
            m_thresholdManual->setChecked(true);
        }
    }

    // selection widget
    {
        auto *selectionWidget = new QWidget(); // NOLINT
        ctrlLayout->addWidget(selectionWidget);
        auto *selectionLayout = new QVBoxLayout(selectionWidget); // NOLINT
        selectionLayout->setContentsMargins(0, 0, 0, 0);

        auto *selectionLabel = new QLabel(tr("Selection")); // NOLINT
        selectionLayout->addWidget(selectionLabel);
        selectionLabel->setFont(QFont("consolas", 16));

        auto *seperator = new QFrame(); // NOLINT
        seperator->setFrameShape(QFrame::HLine);
        seperator->setLineWidth(1);
        selectionLayout->addWidget(seperator);

        m_selectionListView = new QListView();
        selectionLayout->addWidget(m_selectionListView);
        m_selectionListView->setStyleSheet("QListView::item { min-height: 30px; }");
        m_selectionListView->setDragDropMode(QAbstractItemView::InternalMove);
        m_selectionListView->setDefaultDropAction(Qt::MoveAction);
        m_selectionListView->setDragEnabled(true);
        m_selectionListView->setAcceptDrops(true);
        m_selectionListView->setDropIndicatorShown(true);
        m_selectionListView->installEventFilter(this);
        m_selectionModel = new QStandardItemModel();
        m_selectionListView->setModel(m_selectionModel);
        connect(m_selectionModel, &QStandardItemModel::rowsMoved, this, [this] { selectionRequest(); });
        connect(m_selectionModel, &QStandardItemModel::rowsRemoved, this, [this] { selectionRequest(); });
    }

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
}

void AreaSelection::captureRequest(const QString &type, const QString &target) {
    m_type = type;
    m_target = target;
    if (m_type == "screen") {
        // find screen
        QScreen *screen = nullptr;
        for (QScreen *s: QGuiApplication::screens()) {
            if (s->name() == target) {
                screen = s;
                break;
            }
        }
        if (!screen) return;
        m_dpr = screen->devicePixelRatio();
        // screenshot
        m_shot = screen->grabWindow(0);
    } else {
        // find camera
        QCameraDevice cameraDevice;
        for (const QCameraDevice &c: QMediaDevices::videoInputs()) {
            if (c.description() == target) {
                cameraDevice = c;
                break;
            }
        }
        if (cameraDevice.isNull()) return;
        m_dpr = 1;
        // take picture
        const auto camera = new QCamera(cameraDevice, this);
        QMediaCaptureSession captureSession;
        captureSession.setCamera(camera);
        QImageCapture imageCapture;
        captureSession.setImageCapture(&imageCapture);
        QEventLoop loop;
        connect(&imageCapture, &QImageCapture::imageCaptured, this, [this, &loop](int, const QImage &img) {
            m_shot = QPixmap::fromImage(img);
            loop.quit();
        });
        camera->start();
        imageCapture.capture();
        loop.exec();
        camera->stop();
        delete camera;
    }
    processRequest();
}

void AreaSelection::reload(const QJsonObject &config) {
    // load dpr
    m_dpr = config["dpr"].toDouble();
    // load charset string (WIP)
    // QStringList charsetList = config["charset"].toString().split('+');
    // load process (WIP)
    QJsonObject process = config["process"].toObject();
    m_processType = process["processType"].toInt();
    m_processComboBox->setCurrentIndex(m_processType);
    m_processStackedWidget->setCurrentIndex(m_processType);
    switch (m_processType) {
        case 0: break;
        case 2: {
            m_thresholdValue = process["thresholdValue"].toInt();
            m_thresholdSlider->setValue(m_thresholdValue);
            m_thresholdType = process["thresholdType"].toInt();
            m_thresholdTypeComboBox->setCurrentIndex(m_thresholdType & 0b00111);
            switch (m_thresholdType & 0b11000) {
                case 0: m_thresholdManual->setChecked(true);
                    break;
                case 8: m_thresholdOtsu->setChecked(true);
                    break;
                case 16: m_thresholdTriangle->setChecked(true);
                    break;
                default: break;
            }
        }
        break;
        default: break;
    }
    // load area list
    m_selectionModel->clear();
    const QJsonArray areaList = config["areaList"].toArray();
    for (const QJsonValue &value: areaList) {
        QJsonArray areaArray = value.toArray();
        const int x = areaArray[0].toInt();
        const int y = areaArray[1].toInt();
        const int width = areaArray[2].toInt();
        const int height = areaArray[3].toInt();
        const auto physicalRect = QRectF(x, y, width, height);
        const auto logicalRect = QRectF(physicalRect.x() / m_dpr, physicalRect.y() / m_dpr, physicalRect.width() / m_dpr, physicalRect.height() / m_dpr);
        auto *item = new QStandardItem(); // NOLINT
        item->setData(QVariant::fromValue(logicalRect), Qt::UserRole + 1);
        item->setData(QVariant::fromValue(physicalRect), Qt::UserRole + 2);
        item->setFlags(item->flags() &= ~Qt::ItemIsDropEnabled);
        m_selectionModel->appendRow(item);
    }
}

double AreaSelection::dprExport() const {
    return m_dpr;
}

QString AreaSelection::charsetExport() const {
    return m_charsetString;
}

QJsonObject AreaSelection::processExport() const {
    QJsonObject process;
    process["processType"] = m_processType;
    switch (m_processType) {
        case 0:
            break;
        case 1:
            process["kernalSize"] = m_kernalSize;
            break;
        case 2:
            process["thresholdValue"] = m_thresholdValue;
            process["thresholdType"] = m_thresholdType;
            break;
        default:
            break;
    }
    return process;
}

QJsonArray AreaSelection::areaExport() const {
    QJsonArray areaList;
    for (int row = 0; row < m_selectionModel->rowCount(); ++row) {
        const QStandardItem *item = m_selectionModel->item(row);
        const auto physicalRect = item->data(Qt::UserRole + 2).value<QRectF>().toRect();
        QJsonArray areaArray = {physicalRect.x(), physicalRect.y(), physicalRect.width(), physicalRect.height()};
        areaList.append(areaArray);
    }
    return areaList;
}

// AreaSelection protected
bool AreaSelection::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_selectionListView && event->type() == QEvent::KeyPress) {
        switch (static_cast<QKeyEvent *>(event)->key()) {
            case Qt::Key_Delete: {
                const int row = m_selectionListView->currentIndex().row();
                m_selectionModel->removeRow(row);
                return true;
            }
            default:
                break;
        }
    }
    return QDialog::eventFilter(obj, event);
}

// AreaSelection private
void AreaSelection::processRequest() {
    if (m_shot.isNull()) {
        return;
    }
    if (m_processType == RAW) {
        m_pshot = m_shot;
    } else {
        switch (m_processType) {
            case GAUSSIANBLUR: {
                m_pshot = processGaussianBlur(m_shot, m_kernalSize);
                break;
            }
            case THRESHOLD: {
                m_pshot = processThreshold(m_shot, m_thresholdValue, m_thresholdType);
                break;
            }
            default: break;
        }
    }
    m_pshot.setDevicePixelRatio(m_dpr);
    selectionRequest();
}

void AreaSelection::selectionRequest() const {
    m_graphicsScene->clear();
    m_graphicsScene->addPixmap(m_pshot);
    for (int row = 0; row < m_selectionModel->rowCount(); ++row) {
        const QStandardItem *item = m_selectionModel->item(row);
        const auto logicalRect = item->data(Qt::UserRole + 1).value<QRectF>().toRect();
        // gui
        auto *graphicsRectItem = new QGraphicsRectItem(logicalRect); // NOLINT
        m_graphicsScene->addItem(graphicsRectItem);
        graphicsRectItem->setPen(QPen(Qt::red, 2));
        auto *graphicsTextItem = new QGraphicsSimpleTextItem(QString::number(row + 1)); // NOLINT
        m_graphicsScene->addItem(graphicsTextItem);
        graphicsTextItem->setPos(logicalRect.center() - graphicsTextItem->boundingRect().center());
        graphicsTextItem->setBrush(Qt::red);
        graphicsTextItem->setFont(QFont("consolas", 12));
    }
    ocrRequest();
}

void AreaSelection::ocrRequest() const {
    for (int row = 0; row < m_selectionModel->rowCount(); ++row) {
        QStandardItem *item = m_selectionModel->item(row);
        const auto physicalRect = item->data(Qt::UserRole + 2).value<QRectF>().toRect();
        // update ocr result
        const QPixmap cropped = m_pshot.copy(physicalRect);
        const QString recognizedText = ocr(cropped, m_charsetString);
        item->setText(recognizedText);
    }
}

void AreaSelection::charsetRequest() {
    QStringList charsetList;
    for (int row = 0; row < m_charsetModel->rowCount(); ++row) {
        const QStandardItem *item = m_charsetModel->item(row);
        if (item->checkState() == Qt::Checked) {
            const auto lang = item->data(Qt::UserRole + 1).value<QString>();
            charsetList.append(lang);
        }
    }
    m_charsetString = charsetList.join("+");
    ocrRequest();
}