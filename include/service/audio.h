#ifndef UNICOMM_AUDIO_H
#define UNICOMM_AUDIO_H

#include <QByteArray>
#include <QObject>
#include <QString>

struct whisper_context;

class AudioService final : public QObject {
    Q_OBJECT

public:
    explicit AudioService(QObject *parent = nullptr);

    ~AudioService() override;

    [[nodiscard]] QByteArray record();

    [[nodiscard]] QString stt(const QByteArray &pcm) const;

    static void speak(const QString &text);

private:
    whisper_context *m_context{};
    bool m_recording{};
};

#endif //UNICOMM_AUDIO_H
