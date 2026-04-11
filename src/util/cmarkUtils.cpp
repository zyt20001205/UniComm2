#include "util/cmarkUtils.h"

#include <QDebug>

#include <cmark.h>

QString md2html(const QString &md) {
    const QByteArray mdByteArray = md.toUtf8();
    const char *mdChar = mdByteArray.constData();
    // parse to html
    char *htmlChar = cmark_markdown_to_html(mdChar, strlen(mdChar), CMARK_OPT_DEFAULT);
    const QString htmlStr = QString::fromUtf8(htmlChar);
    free(htmlChar);
    // qDebug() << "md:\n" << md << '\n';
    // qDebug() << "md escaped:\n" << md.toHtmlEscaped() << '\n';
    // qDebug() << "html:\n" << htmlStr << '\n';
    // remove <p>             </p>\n
    return htmlStr.mid(3, htmlStr.size() - 7 - 1);
}
