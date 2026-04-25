#include "core/text/textnormalizer.h"

namespace {

QString normalizeLine(QString line, bool ignoreCase, bool trimWhitespace)
{
    if (trimWhitespace)
        line = line.trimmed();
    if (ignoreCase)
        line = line.toCaseFolded();
    return line;
}

}

namespace mergeqt::core {

NormalizedTextDocument TextNormalizer::normalize(const TextDocument &document,
                                                 bool ignoreCase,
                                                 bool trimWhitespace) const
{
    NormalizedTextDocument normalized;
    normalized.label = document.label;
    normalized.sourcePath = document.sourcePath;
    normalized.originalLines = document.lines;
    normalized.normalizedLines.reserve(document.lines.size());

    for (const QString &line : document.lines)
        normalized.normalizedLines.append(normalizeLine(line, ignoreCase, trimWhitespace));

    return normalized;
}

} // namespace mergeqt::core
