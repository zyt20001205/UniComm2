#include "scriptModule/nuspellModule.h"

#include <QCoreApplication>
#include <QDir>
#include <QUrl>
#include <nuspell/finder.hxx>

// NuspellModule public
NuspellModule::NuspellModule(QObject *parent)
    : QObject(parent) {
    const QDir rootDir(QCoreApplication::applicationDirPath());
    QDir dictDir(rootDir.absoluteFilePath("dict"));
    auto dirs = std::vector<std::filesystem::path>{dictDir.path().toStdString()};
    nuspell::append_default_dir_paths(dirs);
    const auto dictPath = nuspell::search_dirs_for_one_dict(dirs, "en_US");
    m_dict.load_aff_dic(dictPath);
}

void NuspellModule::spellCheckRequest(const QUrl &scriptUrl, const QString &script) {
    QVariantList typos{};
    int currentLine = 0;
    // 1: separate script to lines
    const QStringList lines = script.split("\r\n");
    for (const QString &line: lines) {
        // 2: separate words to check spelling
        int currentIndex = 0;
        while (currentIndex < line.length()) {
            QChar ch = line[currentIndex];
            if (!ch.isLetter()) {
                ++currentIndex;
                continue;
            }
            int indexFrom = currentIndex;
            ++currentIndex;
            int indexTo = indexFrom;
            while (currentIndex < line.length() && line[currentIndex].isLower()) {
                indexTo = currentIndex;
                ++currentIndex;
            }
            const QString word = line.mid(indexFrom, indexTo - indexFrom + 1);
            if (!m_dict.spell(word.toStdString())) {
                QVariantMap map = {};
                map["line"] = currentLine;
                map["indexFrom"] = indexFrom;
                map["indexTo"] = indexTo + 1;
                typos.append(map);
            }
        }
        currentLine++;
    }
    emit responseSpellCheck(scriptUrl, typos);
}

QStringList NuspellModule::spellSuggestRequest(const QString &word) const {
    if (word.isEmpty()) return{};
    // send to nuspell
    QStringList suggestions{};
    std::vector<std::string> sugs;
    m_dict.suggest(word.toStdString(), sugs);
    const int count = qMin(sugs.size(), static_cast<size_t>(5));
    for (int i = 0; i < count; ++i) {
        const QString suggestion = QString::fromStdString(sugs[i]);
        suggestions.append(suggestion);
    }
    return suggestions;
}
