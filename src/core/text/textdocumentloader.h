#pragma once

#include "core/text/textcomparetypes.h"

namespace mergeqt::core {

class TextDocumentLoader
{
public:
    [[nodiscard]] TextDocument fromText(const QString &text,
                                        const QString &label = {},
                                        const QString &sourcePath = {},
                                        const QString &encodingName = QStringLiteral("UTF-8")) const;
    [[nodiscard]] bool fromFile(const QString &path, TextDocument &outDocument, QString &errorMessage) const;
};

} // namespace mergeqt::core
