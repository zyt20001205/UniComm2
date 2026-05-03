#ifndef UNICOMM_GLOBALMANAGER_H
#define UNICOMM_GLOBALMANAGER_H

#include <QHash>
#include <QObject>

#include "globals.h"

class GlobalManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(int theme READ themeGet WRITE themeSet NOTIFY themeChanged)
    Q_PROPERTY(QString fore READ foreGet NOTIFY themeChanged)
    Q_PROPERTY(QString foreHover READ foreHoverGet NOTIFY themeChanged)
    Q_PROPERTY(QString forePressed READ forePressedGet NOTIFY themeChanged)
    Q_PROPERTY(QString foreSelected READ foreSelectedGet NOTIFY themeChanged)
    
    Q_PROPERTY(QString back READ backGet NOTIFY themeChanged)
    Q_PROPERTY(QString backHover READ backHoverGet NOTIFY themeChanged)
    Q_PROPERTY(QString backPressed READ backPressedGet NOTIFY themeChanged)
    Q_PROPERTY(QString backSelected READ backSelectedGet NOTIFY themeChanged)

    Q_PROPERTY(QString brandBack READ brandBackGet NOTIFY themeChanged)

    Q_PROPERTY(QString successFore READ successForeGet NOTIFY themeChanged)
    Q_PROPERTY(QString successBack READ successBackGet NOTIFY themeChanged)
    
    Q_PROPERTY(QString warningFore READ warningForeGet NOTIFY themeChanged)
    Q_PROPERTY(QString warningBack READ warningBackGet NOTIFY themeChanged)
    
    Q_PROPERTY(QString dangerFore READ dangerForeGet NOTIFY themeChanged)
    Q_PROPERTY(QString dangerBack READ dangerBackGet NOTIFY themeChanged)

public:
    explicit GlobalManager(QWidget *parent = nullptr);

    ~GlobalManager() override = default;

    [[nodiscard]] int themeGet() const;

    void themeSet(int status);

    [[nodiscard]] QString foreGet() const {
        return m_theme == Theme::Light ? m_palette["lightFore"] : m_palette["darkFore"];
    }

    [[nodiscard]] QString foreHoverGet() const {
        return m_theme == Theme::Light ? m_palette["lightForeHover"] : m_palette["darkForeHover"];
    }

    [[nodiscard]] QString forePressedGet() const {
        return m_theme == Theme::Light ? m_palette["lightForePressed"] : m_palette["darkForePressed"];
    }

    [[nodiscard]] QString foreSelectedGet() const {
        return m_theme == Theme::Light ? m_palette["lightForeSelected"] : m_palette["darkForeSelected"];
    }

    [[nodiscard]] QString backGet() const {
        return m_theme == Theme::Light ? m_palette["lightBack"] : m_palette["darkBack"];
    }

    [[nodiscard]] QString backHoverGet() const {
        return m_theme == Theme::Light ? m_palette["lightBackHover"] : m_palette["darkBackHover"];
    }

    [[nodiscard]] QString backPressedGet() const {
        return m_theme == Theme::Light ? m_palette["lightBackPressed"] : m_palette["darkBackPressed"];
    }

    [[nodiscard]] QString backSelectedGet() const {
        return m_theme == Theme::Light ? m_palette["lightBackSelected"] : m_palette["darkBackSelected"];
    }

    [[nodiscard]] QString brandBackGet() const {
        return m_theme == Theme::Light ? m_palette["lightBrandBack"] : m_palette["darkBrandBack"];
    }

    [[nodiscard]] QString successForeGet() const {
        return m_theme == Theme::Light ? m_palette["lightSuccessFore"] : m_palette["darkSuccessFore"];
    }
    
    [[nodiscard]] QString successBackGet() const {
        return m_theme == Theme::Light ? m_palette["lightSuccessBack"] : m_palette["darkSuccessBack"];
    }
    
    [[nodiscard]] QString warningForeGet() const {
        return m_theme == Theme::Light ? m_palette["lightWarningFore"] : m_palette["darkWarningFore"];
    }
    
    [[nodiscard]] QString warningBackGet() const {
        return m_theme == Theme::Light ? m_palette["lightWarningBack"] : m_palette["darkWarningBack"];
    }
    
    [[nodiscard]] QString dangerForeGet() const {
        return m_theme == Theme::Light ? m_palette["lightDangerFore"] : m_palette["darkDangerFore"];
    }
    
    [[nodiscard]] QString dangerBackGet() const {
        return m_theme == Theme::Light ? m_palette["lightDangerBack"] : m_palette["darkDangerBack"];
    }

signals:
    void themeChanged();

private:
    int m_theme = Theme::Light;
    QHash<QString, QString> m_palette{};
    QStringList m_styleSheet{};
};

#endif //UNICOMM_GLOBALMANAGER_H
