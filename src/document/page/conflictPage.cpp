#include "document/page/conflictPage.h"

#include <QDir>
#include <QFileInfo>
#include <QShortcut>
#include <QGridLayout>

#include "globals.h"
#include "core/globalManager.h"
#include "document/module/conflictWidget.h"
#include "document/module/resolveWidget.h"
#include "document/module/scintillaWidget.h"

// public
ConflictPage::ConflictPage(const QJsonObject &documentConfig, const QUrl &documentUrl)
    : BasePage(documentUrl),
      m_widget(new QWidget(this)),
      m_conflictWidget(new ConflictWidget(documentConfig, documentUrl, m_widget)),
      m_resolveWidget(new ResolveWidget(m_widget)) {
    setIcon(g_globalManager->themeGet() == Theme::Light ? QIcon(":/icon/gitBranchConflictLight.svg") : QIcon(":/icon/gitBranchConflictDark.svg"));
    auto *layout = new QGridLayout(m_widget); // NOLINT
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_conflictWidget, 0, 0);
    layout->addWidget(m_resolveWidget, 0, 0, Qt::AlignTop | Qt::AlignRight);
    setWidget(m_widget);
    connect(m_conflictWidget, &ConflictWidget::appendLog, this, &ConflictPage::appendLog);
    connect(m_conflictWidget, &ConflictWidget::changeSavepoint, this, &ConflictPage::savepointChange);
    connect(m_conflictWidget, &ConflictWidget::changeSelection, this, &ConflictPage::changeSelection);
}

void ConflictPage::propertySet(const QVariantHash &objects) {
    m_saveDialog = qvariant_cast<QObject *>(objects["documentModuleSaveDialog"]);
    m_conflictWidget->propertySet(QVariantHash{
        {"mainWindowToolTip", objects["mainWindowToolTip"]}
    });
    m_resolveWidget->propertySet(QVariantHash{
        {"mainWindowToolTip", objects["mainWindowToolTip"]}
    });
}

void ConflictPage::documentSave() {
    m_conflictWidget->documentSave();
}

bool ConflictPage::documentClose(const bool force) {
    if (force) {
        emit closeDocument(m_documentUrl);
        deleteLater();
        return true;
    }
    bool status = true;
    if (handler()->modifyGet()) {
        m_saveDialog->setProperty("documentUrl", m_documentUrl);
        m_saveDialog->setProperty("documentName", m_documentUrl.fileName());
        QMetaObject::invokeMethod(m_saveDialog, "open");
        const auto eventloop = new QEventLoop(this);
        const auto conn = connect(m_saveDialog, SIGNAL(closed()), eventloop, SLOT(quit()));
        eventloop->exec();
        disconnect(conn);
        delete eventloop;
        status = m_saveDialog->property("status").toBool();
    }
    return status;
}

void ConflictPage::documentGoto() const {
    m_conflictWidget->documentGoto();
}

// private
void ConflictPage::savepointChange(const bool status) {
    const QString pageName = title();
    if (status) {
        setTitle(pageName + "*");
    } else {
        setTitle(pageName.chopped(1));
    }
}
