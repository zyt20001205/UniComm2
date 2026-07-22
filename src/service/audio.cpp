#include "service/audio.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <QAudioSource>
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>
#include <QMediaDevices>
#include <QScopedValueRollback>
#include <QTimer>
#include <QtEndian>

#include <whisper.h>

AudioService::AudioService(QObject *parent)
    : QObject(parent) {
}

AudioService::~AudioService() {
    if (m_context) whisper_free(m_context);
}

QByteArray AudioService::record() {
    constexpr int windowMs = 20;
    constexpr int calibrationMs = 300;
    constexpr int preRollMs = 300;
    constexpr int voiceStartMs = 120;
    constexpr int silenceEndMs = 800;
    constexpr int maximumSpeechMs = 20000;
    constexpr int maximumWaitMs = 30000;
    constexpr float startMarginDb = 12.0f;
    constexpr float endMarginDb = 6.0f;
    constexpr float minimumStartDb = -42.0f;
    constexpr float minimumEndDb = -50.0f;

    if (m_recording) {
        qWarning() << "Audio recording is already active.";
        return {};
    }
    QScopedValueRollback recordingGuard(m_recording, true);

    const auto input = QMediaDevices::defaultAudioInput();
    if (input.isNull()) {
        qWarning() << "No audio input device is available.";
        return {};
    }

    QAudioFormat format{};
    format.setSampleRate(WHISPER_SAMPLE_RATE);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    if (!input.isFormatSupported(format)) {
        qWarning() << "The audio input device does not support 16 kHz mono Int16 PCM.";
        return {};
    }

    QAudioSource source(input, format);
    auto *device = source.start();
    if (!device || source.error() != QtAudio::NoError) {
        qWarning() << "Unable to start audio input:" << source.error();
        return {};
    }

    QEventLoop loop{};
    QTimer timeout{};
    timeout.setSingleShot(true);

    QByteArray pending{};
    QByteArray preRoll{};
    QByteArray recording{};
    float noiseFloor = -60.0f;
    int calibratedFor = 0;
    int voiceFor = 0;
    int silenceFor = 0;
    bool speaking = false;
    bool captured = false;

    const auto levelGet = [&format](const QByteArray &chunk) {
        const auto bytesPerSample = format.bytesPerSample();
        const auto sampleCount = bytesPerSample > 0 ? chunk.size() / bytesPerSample : 0;
        if (sampleCount <= 0) return -120.0f;

        double energy = 0.0;
        const auto *data = chunk.constData();
        for (qsizetype index = 0; index < sampleCount; ++index) {
            const auto sample = std::clamp<double>(format.normalizedSampleValue(data + index * bytesPerSample), -1.0, 1.0);
            energy += sample * sample;
        }

        const auto rms = std::sqrt(energy / static_cast<double>(sampleCount));
        return static_cast<float>(20.0 * std::log10(std::max(rms, 1.0e-6)));
    };

    const auto windowFrames = std::max(1, format.sampleRate() * windowMs / 1000);
    const auto windowBytes = format.bytesForFrames(windowFrames);
    const auto preRollFrames = std::max(1, format.sampleRate() * preRollMs / 1000);
    const auto preRollBytes = format.bytesForFrames(preRollFrames);

    connect(device, &QIODevice::readyRead, &loop, [&] {
        pending.append(device->readAll());
        while (pending.size() >= windowBytes) {
            const auto chunk = pending.first(windowBytes);
            pending.remove(0, windowBytes);
            const auto level = levelGet(chunk);

            if (!speaking) {
                preRoll.append(chunk);
                if (preRoll.size() > preRollBytes) preRoll.remove(0, preRoll.size() - preRollBytes);

                if (calibratedFor < calibrationMs) {
                    if (calibratedFor == 0) noiseFloor = level;
                    else noiseFloor = 0.9f * noiseFloor + 0.1f * level;
                    calibratedFor += windowMs;
                    continue;
                }

                const auto startLevel = std::max(noiseFloor + startMarginDb, minimumStartDb);
                if (level > startLevel) {
                    voiceFor += windowMs;
                } else {
                    voiceFor = 0;
                    noiseFloor = 0.995f * noiseFloor + 0.005f * level;
                }

                if (voiceFor >= voiceStartMs) {
                    speaking = true;
                    recording = preRoll;
                    silenceFor = 0;
                    qDebug() << "Speech started at" << level << "dBFS";
                }
                continue;
            }

            recording.append(chunk);
            const auto endLevel = std::max(noiseFloor + endMarginDb, minimumEndDb);
            if (level < endLevel) silenceFor += windowMs;
            else silenceFor = 0;

            const auto durationMs = static_cast<int>(format.durationForBytes(recording.size()) / 1000);
            if (silenceFor >= silenceEndMs || durationMs >= maximumSpeechMs) {
                captured = true;
                loop.quit();
                break;
            }
        }
    });
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    qDebug() << "Listening for speech:" << format;
    timeout.start(maximumWaitMs);
    loop.exec();
    source.stop();

    if (!captured || recording.isEmpty()) {
        qDebug() << "Speech capture stopped without a complete utterance.";
        return {};
    }

    qDebug() << "Speech captured:" << format.durationForBytes(recording.size()) / 1000 << "ms";
    return recording;
}

QString AudioService::stt(const QByteArray &pcm) {
    if (pcm.isEmpty() || pcm.size() % static_cast<qsizetype>(sizeof(qint16)) != 0) return {};
    if (!contextEnsure()) return {};

    const auto sampleCount = pcm.size() / static_cast<qsizetype>(sizeof(qint16));
    std::vector<float> samples(static_cast<size_t>(sampleCount));
    for (qsizetype index = 0; index < sampleCount; ++index) {
        const auto sample = qFromLittleEndian<qint16>(pcm.constData() + index * sizeof(qint16));
        samples[static_cast<size_t>(index)] = static_cast<float>(sample) / 32768.0f;
    }

    auto params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.language = "zh";
    params.detect_language = false;
    params.translate = false;
    params.no_context = true;
    params.no_timestamps = true;
    params.print_special = false;
    params.print_progress = false;
    params.print_realtime = false;
    params.print_timestamps = false;

    if (whisper_full(m_context, params, samples.data(), static_cast<int>(samples.size())) != 0) {
        qWarning() << "Whisper transcription failed.";
        return {};
    }

    QString text{};
    const auto segmentCount = whisper_full_n_segments(m_context);
    for (int index = 0; index < segmentCount; ++index) {
        const auto *segment = whisper_full_get_segment_text(m_context, index);
        if (segment) text.append(QString::fromUtf8(segment));
    }
    return text.trimmed();
}

bool AudioService::contextEnsure() {
    // https://huggingface.co/ggerganov/whisper.cpp/tree/main
    if (m_context) return true;

    const auto applicationDir = QCoreApplication::applicationDirPath();
    QLibrary ggml(QDir(applicationDir).filePath("ggml.dll"));
    if (!ggml.load()) {
        qWarning() << "Unable to load ggml.dll:" << ggml.errorString();
        return false;
    }

    using BackendLoadAllFromPath = void (*)(const char *);
    using BackendCount = size_t (*)();
    const auto backendLoadAllFromPath = reinterpret_cast<BackendLoadAllFromPath>(ggml.resolve("ggml_backend_load_all_from_path"));
    const auto backendRegCount = reinterpret_cast<BackendCount>(ggml.resolve("ggml_backend_reg_count"));
    const auto backendDevCount = reinterpret_cast<BackendCount>(ggml.resolve("ggml_backend_dev_count"));
    if (!backendLoadAllFromPath) {
        qWarning() << "Unable to resolve ggml_backend_load_all_from_path.";
        return false;
    }

    const auto encodedApplicationDir = QFile::encodeName(applicationDir);
    backendLoadAllFromPath(encodedApplicationDir.constData());
    if (backendRegCount && backendDevCount) {
        qDebug() << "GGML backends loaded:" << backendRegCount() << "devices:" << backendDevCount();
    }

    const auto modelPath = QDir(QCoreApplication::applicationDirPath()).filePath("whisper/model/ggml-base.bin");
    if (!QFileInfo(modelPath).isFile()) {
        qWarning() << "Whisper model was not found:" << modelPath;
        return false;
    }

    auto params = whisper_context_default_params();
    params.use_gpu = true;
    params.flash_attn = false;
    const auto encodedPath = QFile::encodeName(modelPath);
    m_context = whisper_init_from_file_with_params(encodedPath.constData(), params);
    if (!m_context) {
        qWarning() << "Unable to load the Whisper model:" << modelPath;
        return false;
    }
    return true;
}
