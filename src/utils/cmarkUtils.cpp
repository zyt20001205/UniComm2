#include "utils/cmarkUtils.h"

#include "cmark.h"

QString md2html(const QString &md) {
    const QByteArray mdByteArray = md.toHtmlEscaped().toUtf8();
    const char *mdChar = mdByteArray.constData();
    // parse to html
    char *htmlChar = cmark_markdown_to_html(mdChar, strlen(mdChar), CMARK_OPT_DEFAULT);
    const QString htmlStr = QString::fromUtf8(htmlChar);
    // remove <p>             </p>\n
    return htmlStr.mid(3, htmlStr.size() - 7 - 1);
}
