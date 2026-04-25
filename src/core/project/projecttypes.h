#pragma once

#include <QString>

namespace mergeqt::core {

enum class CompareScope
{
    Text,
    File,
    Folder
};

struct CompareTarget
{
    QString leftPath;
    QString rightPath;
    CompareScope scope = CompareScope::Text;

    [[nodiscard]] bool isValid() const
    {
        return !leftPath.isEmpty() && !rightPath.isEmpty();
    }
};

} // namespace mergeqt::core
