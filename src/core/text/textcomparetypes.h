#pragma once

#include <QString>
#include <QVector>

namespace mergeqt::core {

enum class NewlineStyle
{
    None,
    LF,
    CRLF,
    CR,
    Mixed
};

enum class TextDiffOperation
{
    Equal,
    Insert,
    Delete,
    Replace
};

struct TextDiffLine
{
    int leftLineNumber = -1;
    int rightLineNumber = -1;
    QString leftText;
    QString rightText;
    TextDiffOperation operation = TextDiffOperation::Equal;

    [[nodiscard]] bool isDifference() const
    {
        return operation != TextDiffOperation::Equal;
    }
};

struct TextDiffBlock
{
    int startIndex = -1;
    int endIndex = -1;
    int firstLeftLineNumber = -1;
    int lastLeftLineNumber = -1;
    int firstRightLineNumber = -1;
    int lastRightLineNumber = -1;
    TextDiffOperation operation = TextDiffOperation::Equal;
    int lineCount = 0;

    [[nodiscard]] bool isDifference() const
    {
        return operation != TextDiffOperation::Equal;
    }
};

struct TextCompareStats
{
    int equalCount = 0;
    int insertCount = 0;
    int deleteCount = 0;
    int replaceCount = 0;

    [[nodiscard]] int differenceCount() const
    {
        return insertCount + deleteCount + replaceCount;
    }
};

struct TextCompareInput
{
    QString leftText;
    QString rightText;
    bool ignoreCase = false;
    bool trimWhitespace = false;
    QString leftLabel;
    QString rightLabel;
};

struct TextDocument
{
    QString text;
    QString label;
    QString sourcePath;
    QStringList lines;
    QString encodingName;
    NewlineStyle newlineStyle = NewlineStyle::None;
};

struct NormalizedTextDocument
{
    QString label;
    QString sourcePath;
    QStringList originalLines;
    QStringList normalizedLines;
};

struct TextCompareResult
{
    QVector<TextDiffLine> lines;
    QVector<TextDiffBlock> blocks;
    TextCompareStats stats;

    [[nodiscard]] bool hasDifferences() const
    {
        return stats.differenceCount() > 0;
    }
};

} // namespace mergeqt::core
