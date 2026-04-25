#include "core/text/textdocumentloader.h"

#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

namespace {

QStringList splitLinesKeepingEmpty(const QString &text)
{
    return text.split(u'\n');
}

mergeqt::core::NewlineStyle detectNewlineStyle(const QByteArray &bytes)
{
    bool sawLf = false;
    bool sawCrLf = false;
    bool sawCr = false;

    for (int i = 0; i < bytes.size(); ++i) {
        if (bytes.at(i) == '\r') {
            if (i + 1 < bytes.size() && bytes.at(i + 1) == '\n') {
                sawCrLf = true;
                ++i;
            } else {
                sawCr = true;
            }
        } else if (bytes.at(i) == '\n') {
            sawLf = true;
        }
    }

    const int styles = int(sawLf) + int(sawCrLf) + int(sawCr);
    if (styles > 1)
        return mergeqt::core::NewlineStyle::Mixed;
    if (sawCrLf)
        return mergeqt::core::NewlineStyle::CRLF;
    if (sawLf)
        return mergeqt::core::NewlineStyle::LF;
    if (sawCr)
        return mergeqt::core::NewlineStyle::CR;
    return mergeqt::core::NewlineStyle::None;
}

QString normalizeNewlines(QString text)
{
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QChar(u'\r'), QChar(u'\n'));
    return text;
}

QString decodeBytes(const QByteArray &bytes, QString &encodingName)
{
    if (bytes.startsWith("\xEF\xBB\xBF")) {
        encodingName = QStringLiteral("UTF-8 BOM");
        return QString::fromUtf8(bytes.sliced(3));
    }

    if (bytes.size() >= 2) {
        const uchar b0 = static_cast<uchar>(bytes.at(0));
        const uchar b1 = static_cast<uchar>(bytes.at(1));
        if (b0 == 0xFF && b1 == 0xFE) {
            encodingName = QStringLiteral("UTF-16 LE");
            QStringDecoder decoder(QStringDecoder::Utf16LE);
            return decoder.decode(bytes.sliced(2));
        }
        if (b0 == 0xFE && b1 == 0xFF) {
            encodingName = QStringLiteral("UTF-16 BE");
            QStringDecoder decoder(QStringDecoder::Utf16BE);
            return decoder.decode(bytes.sliced(2));
        }
    }

    encodingName = QStringLiteral("UTF-8");
    return QString::fromUtf8(bytes);
}

} // namespace

namespace mergeqt::core {

TextDocument TextDocumentLoader::fromText(const QString &text,
                                          const QString &label,
                                          const QString &sourcePath,
                                          const QString &encodingName) const
{
    TextDocument document;
    document.text = normalizeNewlines(text);
    document.label = label;
    document.sourcePath = sourcePath;
    document.lines = splitLinesKeepingEmpty(document.text);
    document.encodingName = encodingName;
    document.newlineStyle = text.contains(QStringLiteral("\r\n")) ? NewlineStyle::CRLF
                            : text.contains(QChar(u'\n')) ? NewlineStyle::LF
                            : text.contains(QChar(u'\r')) ? NewlineStyle::CR
                                                          : NewlineStyle::None;
    return document;
}

bool TextDocumentLoader::fromFile(const QString &path, TextDocument &outDocument, QString &errorMessage) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = QObject::tr("Failed to open file: %1").arg(path);
        return false;
    }

    const QByteArray bytes = file.readAll();
    QString encodingName;
    QString text = decodeBytes(bytes, encodingName);
    outDocument = fromText(text, QFileInfo(path).fileName(), path, encodingName);
    outDocument.newlineStyle = detectNewlineStyle(bytes);
    return true;
}

} // namespace mergeqt::core
