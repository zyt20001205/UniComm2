#ifndef UNICOMM_GLOBALMANAGER_H
#define UNICOMM_GLOBALMANAGER_H

#include <QHash>
#include <QObject>

#include "globals.h"

class GlobalManager final : public QObject {
    Q_OBJECT
    Q_PROPERTY(int theme READ themeGet CONSTANT)
    Q_PROPERTY(QString fore READ foreGet CONSTANT)
    Q_PROPERTY(QString foreHover READ foreHoverGet CONSTANT)
    Q_PROPERTY(QString forePressed READ forePressedGet CONSTANT)
    Q_PROPERTY(QString foreSelected READ foreSelectedGet CONSTANT)

    Q_PROPERTY(QString back READ backGet CONSTANT)
    Q_PROPERTY(QString backHover READ backHoverGet CONSTANT)
    Q_PROPERTY(QString backPressed READ backPressedGet CONSTANT)
    Q_PROPERTY(QString backSelected READ backSelectedGet CONSTANT)

    Q_PROPERTY(QString stroke READ strokeGet CONSTANT)
    Q_PROPERTY(QString strokePressed READ strokePressedGet CONSTANT)

    Q_PROPERTY(QString brandFore READ brandForeGet CONSTANT)
    Q_PROPERTY(QString brandBack READ brandBackGet CONSTANT)

    Q_PROPERTY(QString successFore2 READ successFore2Get CONSTANT)
    Q_PROPERTY(QString successBack2 READ successBack2Get CONSTANT)
    Q_PROPERTY(QString successFore3 READ successFore3Get CONSTANT)
    Q_PROPERTY(QString successBack3 READ successBack3Get CONSTANT)

    Q_PROPERTY(QString warningFore2 READ warningFore2Get CONSTANT)
    Q_PROPERTY(QString warningBack2 READ warningBack2Get CONSTANT)
    Q_PROPERTY(QString warningFore3 READ warningFore3Get CONSTANT)
    Q_PROPERTY(QString warningBack3 READ warningBack3Get CONSTANT)

    Q_PROPERTY(QString dangerFore2 READ dangerFore2Get CONSTANT)
    Q_PROPERTY(QString dangerBack2 READ dangerBack2Get CONSTANT)
    Q_PROPERTY(QString dangerFore3 READ dangerFore3Get CONSTANT)
    Q_PROPERTY(QString dangerBack3 READ dangerBack3Get CONSTANT)

    // watch module
    Q_PROPERTY(bool refresh READ refreshGet WRITE refreshSet)

public:
    explicit GlobalManager(QWidget *parent = nullptr);

    ~GlobalManager() override = default;

    [[nodiscard]] int themeGet() const {
        return m_theme;
    }

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

    [[nodiscard]] QString strokeGet() const {
        return m_theme == Theme::Light ? m_palette["lightStroke"] : m_palette["darkStroke"];
    }

    [[nodiscard]] QString strokePressedGet() const {
        return m_theme == Theme::Light ? m_palette["lightStrokePressed"] : m_palette["darkStrokePressed"];
    }

    [[nodiscard]] QString brandForeGet() const {
        return m_theme == Theme::Light ? m_palette["lightBrandFore"] : m_palette["darkBrandFore"];
    }

    [[nodiscard]] QString brandBackGet() const {
        return m_theme == Theme::Light ? m_palette["lightBrandBack"] : m_palette["darkBrandBack"];
    }
    
    [[nodiscard]] QString successFore2Get() const {
        return m_theme == Theme::Light ? m_palette["lightSuccessFore2"] : m_palette["darkSuccessFore2"];
    }

    [[nodiscard]] QString successBack2Get() const {
        return m_theme == Theme::Light ? m_palette["lightSuccessBack2"] : m_palette["darkSuccessBack2"];
    }
    
    [[nodiscard]] QString successFore3Get() const {
        return m_theme == Theme::Light ? m_palette["lightSuccessFore3"] : m_palette["darkSuccessFore3"];
    }

    [[nodiscard]] QString successBack3Get() const {
        return m_theme == Theme::Light ? m_palette["lightSuccessBack3"] : m_palette["darkSuccessBack3"];
    }
    
    [[nodiscard]] QString warningFore2Get() const {
        return m_theme == Theme::Light ? m_palette["lightWarningFore2"] : m_palette["darkWarningFore2"];
    }

    [[nodiscard]] QString warningBack2Get() const {
        return m_theme == Theme::Light ? m_palette["lightWarningBack2"] : m_palette["darkWarningBack2"];
    }

    [[nodiscard]] QString warningFore3Get() const {
        return m_theme == Theme::Light ? m_palette["lightWarningFore3"] : m_palette["darkWarningFore3"];
    }

    [[nodiscard]] QString warningBack3Get() const {
        return m_theme == Theme::Light ? m_palette["lightWarningBack3"] : m_palette["darkWarningBack3"];
    }

    [[nodiscard]] QString dangerFore2Get() const {
        return m_theme == Theme::Light ? m_palette["lightDangerFore2"] : m_palette["darkDangerFore2"];
    }

    [[nodiscard]] QString dangerBack2Get() const {
        return m_theme == Theme::Light ? m_palette["lightDangerBack2"] : m_palette["darkDangerBack2"];
    }
    
    [[nodiscard]] QString dangerFore3Get() const {
        return m_theme == Theme::Light ? m_palette["lightDangerFore3"] : m_palette["darkDangerFore3"];
    }

    [[nodiscard]] QString dangerBack3Get() const {
        return m_theme == Theme::Light ? m_palette["lightDangerBack3"] : m_palette["darkDangerBack3"];
    }

    [[nodiscard]] bool refreshGet() const {
        return m_refresh;
    }

    void refreshSet(const bool refresh) {
        m_refresh = refresh;
    }

private:
    int m_theme{};
    QHash<QString, QString> m_palette{};
    QStringList m_styleSheet{};
    bool m_refresh{};
};

#endif //UNICOMM_GLOBALMANAGER_H
