#ifndef UNICOMM_SMTP_H
#define UNICOMM_SMTP_H

#include <QObject>
#include <sol/object.hpp>

class BasePort;

class Smtp final : public QObject {
    Q_OBJECT

public:
    explicit Smtp(QObject *parent = nullptr);

    ~Smtp() override = default;

    void init(const std::string &portName, int timeout);

    void authLogin(const std::string &username, const std::string &password) const;

    void ehlo() const;

    void _send(const std::string &from, const sol::object &to, const sol::object &cc, const sol::object &bcc, const std::string &subject, const std::string &body,
              const std::string &attachment) const;

    void send(const std::string &from, const sol::object &to, const sol::object &cc, const sol::object &bcc, const std::string &subject, const std::string &body,
              const sol::optional<std::string> &attachment) const;

    void quit() const;

private:
    [[nodiscard]] static QString parser(const QByteArray &rxData);

    std::string m_portName{};
    int m_timeout{};
    BasePort *m_port{};

    struct StatusCode {
        // https://www.iana.org/assignments/smtp-enhanced-status-codes/smtp-enhanced-status-codes.xhtml#smtp-enhanced-status-codes-3
        enum {
            // 2xx Positive Completion
            SystemStatus = 211,
            HelpMessage = 214,
            ServiceReady = 220,
            ServiceClosingTransmissionChannel = 221,
            AuthenticationSucceeded = 235,
            RequestedMailActionOkay = 250,
            UserNotLocalWillForward = 251,
            CannotVerifyUserWillAcceptMessage = 252,
            PendingMessagesForNodeStarted = 253,

            // 3xx Positive Intermediate
            AuthenticationChallenge = 334,
            StartMailInput = 354,

            // 4xx Transient Negative Completion
            PasswordTransitionNeeded = 422,
            InvalidCommand = 430,
            PasswordTransitionRequired = 432,
            MailboxUnavailable = 450,
            LocalErrorInProcessing = 451,
            InsufficientSystemStorage = 452,
            SystemNotAcceptingNetworkMessages = 453,
            TemporaryAuthenticationFailure = 454,
            ServerUnableToAccommodateParameters = 455,
            UnableToQueueMessagesForNode = 458,
            NodeNotAllowed = 459,

            // 5xx Permanent Negative Completion
            SyntaxError = 500,
            SyntaxErrorInParameters = 501,
            CommandNotImplemented = 502,
            BadSequenceOfCommands = 503,
            CommandParameterNotImplemented = 504,
            HostDoesNotAcceptMail = 521,
            EncryptionNeeded = 523,
            EncryptionRequired = 524,
            UserAccountDisabled = 525,
            AuthenticationRequired = 530,
            DeliveryNotAuthorized = 533,
            AuthenticationMechanismTooWeak = 534,
            AuthenticationCredentialsInvalid = 535,
            EncryptionRequiredForAuthenticationMechanism = 538,
            RequestedActionNotTaken = 550,
            UserNotLocal = 551,
            ExceededStorageAllocation = 552,
            MailboxNameNotAllowed = 553,
            TransactionFailed = 554,
            MailParametersNotRecognized = 555,
            DomainDoesNotAcceptMail = 556
        };
    };
};

#endif //UNICOMM_SMTP_H
