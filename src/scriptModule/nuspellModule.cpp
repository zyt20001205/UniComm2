#include "scriptModule/nuspellModule.h"

#include <QCoreApplication>
#include <QDir>
#include <nuspell/finder.hxx>

// NuspellModule public
NuspellModule::NuspellModule(QWidget *parent)
    : QWidget(parent) {
    const QDir rootPath(QCoreApplication::applicationDirPath());
    auto dirs = std::vector<std::filesystem::path>{rootPath.filePath("/dict").toStdString()};
    nuspell::append_default_dir_paths(dirs);
    const auto dictPath = nuspell::search_dirs_for_one_dict(dirs, "en_US");
    m_dict.load_aff_dic(dictPath);
}
