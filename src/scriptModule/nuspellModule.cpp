#include "scriptModule/nuspellModule.h"

#include <QCoreApplication>
#include <QDir>
#include <QUrl>
#include <nuspell/finder.hxx>

// NuspellModule public
NuspellModule::NuspellModule(QWidget *parent)
    : QWidget(parent) {
    const QDir rootDir(QCoreApplication::applicationDirPath());
    QDir dictDir(rootDir.absoluteFilePath("dict"));
    auto dirs = std::vector<std::filesystem::path>{dictDir.path().toStdString()};
    nuspell::append_default_dir_paths(dirs);
    const auto dictPath = nuspell::search_dirs_for_one_dict(dirs, "en_US");
    m_dict.load_aff_dic(dictPath);

    // try {
    //     const QString testWord = "speling";          // 故意写错
    //     auto sugs = std::vector<std::string>();
    //     bool correct = m_dict.spell(testWord.toStdString());
    //
    //     qDebug() << "spelling of" << testWord << "->" << (correct ? "OK" : "MISS");
    //
    //     if (!correct) {
    //         m_dict.suggest(testWord.toStdString(), sugs);
    //         qDebug() << "suggestions:";
    //         for (auto &s : sugs)
    //             qDebug() << QString::fromStdString(s);
    //     }
    // } catch (const std::exception &e) {
    //     qWarning() << "Nuspell test failed:" << e.what();
    // }
}

void NuspellModule::spellCheckFileRequest(const QUrl &scriptUrl) {
    qDebug() << scriptUrl.toString();
}

void NuspellModule::spellCheckWordRequest(const QString &word) const {
    // type check
    int wordType = PLAIN;
    QString wordPlain = word;
    bool isCamel = false;
    for (int i = 1; i < word.size(); ++i) {
        if (word[i - 1].isLower() && word[i].isUpper()) {
            isCamel = true;
            break;
        }
    }
    if (isCamel) {
        wordType = word[0].isUpper() ? UPPERCAMEL : LOWERCAMEL;
        wordPlain = word.toLower();
        return; // WIP
    }

    const QStringList suggestions = spellCheck(word);
    if (!suggestions.isEmpty()) {
        qDebug() << suggestions;
    }
}

// NuspellModule private
QStringList NuspellModule::spellCheck(const QString &word) const {
    if (word.isEmpty()) return {};
    // send to nuspell
    QStringList suggestions{};
    if (!m_dict.spell(word.toStdString())) {
        std::vector<std::string> sugs;
        m_dict.suggest(word.toStdString(), sugs);
        const int count = qMin(sugs.size(), static_cast<size_t>(5));
        for (int i = 0; i < count; ++i) {
            const QString suggestion = QString::fromStdString(sugs[i]);
            suggestions.append(suggestion);
        }
    }
    return suggestions;
}
