#ifndef UNICOMM_TEXTPAGE_H
#define UNICOMM_TEXTPAGE_H

#include <QJsonObject>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>

class SearchWidget;
class ReplaceWidget;
class ScintillaWidget;

class TextPage final : public KDDockWidgets::QtWidgets::DockWidget {
    Q_OBJECT

public:
    explicit TextPage(const QJsonObject &documentConfig = QJsonObject(), const QUrl &documentUrl = QUrl());

    ~TextPage() override = default;

    void propertySet(const QVariantMap &objects);

    // public: file
    void pathDisambiguation();

    void documentReload();

    void documentSave();

    void documentClose();

    QUrl m_documentUrl{};
    ScintillaWidget *m_editorWidget{};

signals:
    void appendLog(const QString &message, int type);

    void closeDocument(const QUrl &documentUrl);

    void changeSelection(const QHash<QString, int> &selection);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void selectionChange();

    void savepointChange(bool status);

    // private: file
    void permissionGet();

    void permissionSet() const;

    // private: search
    void searchToggle();

    void replaceToggle();

    void searchRequest(const QString &text);

    void searchResponse();

    void searchPrev();

    void searchNext();

    void searchClear();

    void textReplace(const QString &text);

    void allReplace(const QString &text);

    SearchWidget *m_searchWidget{};
    ReplaceWidget *m_replaceWidget{};

    QTimer *m_selectionTimer{};

    QObject *m_toolTip{};
    QObject *m_systemPropertyDialog{};

    QHash<QString, int> m_selection{};
    QVariantHash m_search{};
};

#endif //UNICOMM_TEXTPAGE_H
