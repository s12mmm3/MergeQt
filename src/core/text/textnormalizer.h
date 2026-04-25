#pragma once

#include "core/text/textcomparetypes.h"

namespace mergeqt::core {

class TextNormalizer
{
public:
    [[nodiscard]] NormalizedTextDocument normalize(const TextDocument &document,
                                                   bool ignoreCase,
                                                   bool trimWhitespace) const;
};

} // namespace mergeqt::core
