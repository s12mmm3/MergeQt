#pragma once

#include "core/text/textcomparetypes.h"

namespace mergeqt::core {

class ITextDiffEngine
{
public:
    virtual ~ITextDiffEngine() = default;
    [[nodiscard]] virtual TextCompareResult compare(const NormalizedTextDocument &leftDocument,
                                                    const NormalizedTextDocument &rightDocument) const = 0;
};

class MinimalLineDiffEngine final : public ITextDiffEngine
{
public:
    [[nodiscard]] TextCompareResult compare(const NormalizedTextDocument &leftDocument,
                                            const NormalizedTextDocument &rightDocument) const override;
};

} // namespace mergeqt::core
