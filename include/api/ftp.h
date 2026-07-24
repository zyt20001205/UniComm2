#ifndef UNICOMM_FTP_H
#define UNICOMM_FTP_H

#include <QVariant>
#include <sol/table.hpp>

#include <string>

class TcpServer;

class Ftp final : public QObject {
    Q_OBJECT

public:
    explicit Ftp(QObject *parent = nullptr);

    ~Ftp() override = default;

    void init(const std::string &portName, int timeout);

    void start(const sol::table &options);

private:
    struct StatusCode {
        // https://www.iana.org/assignments/ftp-commands-extensions/ftp-commands-extensions.xhtml#ftp-commands-extensions-2
        enum {
            // 1xx Positive Preliminary
            RestartMarkerReply = 110,
            ServiceReadyInMinutes = 120,
            DataConnectionAlreadyOpen = 125,
            FileStatusOkay = 150,

            // 2xx Positive Completion
            CommandOkay = 200,
            CommandNotImplementedSuperfluous = 202,
            SystemStatus = 211,
            DirectoryStatus = 212,
            FileStatus = 213,
            HelpMessage = 214,
            SystemType = 215,
            ServiceReady = 220,
            ServiceClosingControlConnection = 221,
            DataConnectionOpen = 225,
            ClosingDataConnection = 226,
            EnteringPassiveMode = 227,
            UserLoggedIn = 230,
            RequestedFileActionOkay = 250,
            PathnameCreated = 257,

            // 3xx Positive Intermediate
            UserNameOkay = 331,
            NeedAccountForLogin = 332,
            RequestedFileActionPending = 350,

            // 4xx Transient Negative Completion
            ServiceNotAvailable = 421,
            CannotOpenDataConnection = 425,
            ConnectionClosedTransferAborted = 426,
            RequestedFileActionNotTaken = 450,
            RequestedActionAborted = 451,
            InsufficientStorage = 452,

            // 5xx Permanent Negative Completion
            SyntaxError = 500,
            SyntaxErrorInParameters = 501,
            CommandNotImplemented = 502,
            BadSequenceOfCommands = 503,
            CommandNotImplementedForParameter = 504,
            NotLoggedIn = 530,
            NeedAccountForStoringFiles = 532,
            FileUnavailable = 550,
            PageTypeUnknown = 551,
            ExceededStorageAllocation = 552,
            FileNameNotAllowed = 553
        };
    };

    void accept();

    void stateSet(int state);

    [[nodiscard]] static QVariantHash parser(const QByteArray &rxData);

    [[nodiscard]] static QByteArray assembler(int statusCode);

    std::string m_portName{};
    int m_timeout{};
    QString m_peer{};
    TcpServer *m_port{};
    int m_state{StatusCode::ServiceReady};
};

#endif //UNICOMM_FTP_H
