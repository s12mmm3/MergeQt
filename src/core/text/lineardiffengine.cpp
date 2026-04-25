#include "core/text/lineardiffengine.h"

#include <algorithm>

namespace {

using namespace mergeqt::core;

void appendStat(TextCompareStats &stats, TextDiffOperation operation)
{
    switch (operation) {
    case TextDiffOperation::Equal:
        ++stats.equalCount;
        break;
    case TextDiffOperation::Insert:
        ++stats.insertCount;
        break;
    case TextDiffOperation::Delete:
        ++stats.deleteCount;
        break;
    case TextDiffOperation::Replace:
        ++stats.replaceCount;
        break;
    }
}

QVector<TextDiffBlock> buildBlocks(const QVector<TextDiffLine> &lines)
{
    QVector<TextDiffBlock> blocks;
    if (lines.isEmpty())
        return blocks;

    auto flushBlock = [&blocks](TextDiffBlock &block) {
        if (block.startIndex >= 0)
            blocks.append(block);
        block = TextDiffBlock{};
    };

    TextDiffBlock currentBlock;
    for (int index = 0; index < lines.size(); ++index) {
        const TextDiffLine &line = lines.at(index);
        if (currentBlock.startIndex < 0) {
            currentBlock.startIndex = index;
            currentBlock.endIndex = index;
            currentBlock.operation = line.operation;
            currentBlock.firstLeftLineNumber = line.leftLineNumber;
            currentBlock.lastLeftLineNumber = line.leftLineNumber;
            currentBlock.firstRightLineNumber = line.rightLineNumber;
            currentBlock.lastRightLineNumber = line.rightLineNumber;
            currentBlock.lineCount = 1;
            continue;
        }

        if (currentBlock.operation != line.operation) {
            flushBlock(currentBlock);
            currentBlock.startIndex = index;
            currentBlock.endIndex = index;
            currentBlock.operation = line.operation;
            currentBlock.firstLeftLineNumber = line.leftLineNumber;
            currentBlock.lastLeftLineNumber = line.leftLineNumber;
            currentBlock.firstRightLineNumber = line.rightLineNumber;
            currentBlock.lastRightLineNumber = line.rightLineNumber;
            currentBlock.lineCount = 1;
            continue;
        }

        currentBlock.endIndex = index;
        currentBlock.lineCount += 1;
        if (line.leftLineNumber > 0) {
            if (currentBlock.firstLeftLineNumber < 0)
                currentBlock.firstLeftLineNumber = line.leftLineNumber;
            currentBlock.lastLeftLineNumber = line.leftLineNumber;
        }
        if (line.rightLineNumber > 0) {
            if (currentBlock.firstRightLineNumber < 0)
                currentBlock.firstRightLineNumber = line.rightLineNumber;
            currentBlock.lastRightLineNumber = line.rightLineNumber;
        }
    }

    flushBlock(currentBlock);
    return blocks;
}

} // namespace

namespace mergeqt::core {

TextCompareResult MinimalLineDiffEngine::compare(const NormalizedTextDocument &leftDocument,
                                                 const NormalizedTextDocument &rightDocument) const
{
    const QStringList &leftLines = leftDocument.originalLines;
    const QStringList &rightLines = rightDocument.originalLines;
    const QStringList &leftNormalizedLines = leftDocument.normalizedLines;
    const QStringList &rightNormalizedLines = rightDocument.normalizedLines;

    const int leftCount = leftLines.size();
    const int rightCount = rightLines.size();

    QVector<QVector<int>> lcs(leftCount + 1, QVector<int>(rightCount + 1, 0));
    for (int i = leftCount - 1; i >= 0; --i) {
        for (int j = rightCount - 1; j >= 0; --j) {
            if (leftNormalizedLines.at(i) == rightNormalizedLines.at(j))
                lcs[i][j] = lcs[i + 1][j + 1] + 1;
            else
                lcs[i][j] = std::max(lcs[i + 1][j], lcs[i][j + 1]);
        }
    }

    TextCompareResult result;
    int i = 0;
    int j = 0;
    while (i < leftCount && j < rightCount) {
        if (leftNormalizedLines.at(i) == rightNormalizedLines.at(j)) {
            TextDiffLine line;
            line.leftLineNumber = i + 1;
            line.rightLineNumber = j + 1;
            line.leftText = leftLines.at(i);
            line.rightText = rightLines.at(j);
            line.operation = TextDiffOperation::Equal;
            result.lines.append(line);
            appendStat(result.stats, line.operation);
            ++i;
            ++j;
            continue;
        }

        if (i + 1 < leftCount && j + 1 < rightCount
            && leftNormalizedLines.at(i + 1) == rightNormalizedLines.at(j + 1)) {
            TextDiffLine line;
            line.leftLineNumber = i + 1;
            line.rightLineNumber = j + 1;
            line.leftText = leftLines.at(i);
            line.rightText = rightLines.at(j);
            line.operation = TextDiffOperation::Replace;
            result.lines.append(line);
            appendStat(result.stats, line.operation);
            ++i;
            ++j;
            continue;
        }

        if (lcs[i + 1][j] >= lcs[i][j + 1]) {
            TextDiffLine line;
            line.leftLineNumber = i + 1;
            line.leftText = leftLines.at(i);
            line.operation = TextDiffOperation::Delete;
            result.lines.append(line);
            appendStat(result.stats, line.operation);
            ++i;
        } else {
            TextDiffLine line;
            line.rightLineNumber = j + 1;
            line.rightText = rightLines.at(j);
            line.operation = TextDiffOperation::Insert;
            result.lines.append(line);
            appendStat(result.stats, line.operation);
            ++j;
        }
    }

    while (i < leftCount) {
        TextDiffLine line;
        line.leftLineNumber = i + 1;
        line.leftText = leftLines.at(i);
        line.operation = TextDiffOperation::Delete;
        result.lines.append(line);
        appendStat(result.stats, line.operation);
        ++i;
    }

    while (j < rightCount) {
        TextDiffLine line;
        line.rightLineNumber = j + 1;
        line.rightText = rightLines.at(j);
        line.operation = TextDiffOperation::Insert;
        result.lines.append(line);
        appendStat(result.stats, line.operation);
        ++j;
    }

    result.blocks = buildBlocks(result.lines);
    return result;
}

} // namespace mergeqt::core
