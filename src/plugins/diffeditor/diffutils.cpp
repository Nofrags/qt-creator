namespace Internal {
ChunkData calculateOriginalData(const QList<Diff> &leftDiffList,
    ChunkData chunkData;

            while (line < qMax(newLeftLines.count(), newRightLines.count())) {
                handleLine(newLeftLines, line, &leftLines, &leftLineNumber);
                handleLine(newRightLines, line, &rightLines, &rightLineNumber);

                const int commonLineCount = qMin(newLeftLines.count(), newRightLines.count());
                if (line < commonLineCount) {
                    // try to align
                    const int leftDifference = leftLineNumber - leftLineAligned;
                    const int rightDifference = rightLineNumber - rightLineAligned;

                    if (leftDifference && rightDifference) {
                        bool doAlign = true;
                        if (line == 0 // omit alignment when first lines of equalities are empty and last generated lines are not equal
                                && (newLeftLines.at(0).isEmpty() || newRightLines.at(0).isEmpty())
                                && !lastLineEqual) {
                            doAlign = false;
                        }

                        if (line == commonLineCount - 1) {
                            // omit alignment when last lines of equalities are empty
                            if (leftLines.last().text.isEmpty() || rightLines.last().text.isEmpty())

                            // unless it's the last dummy line (don't omit in that case)
                            if (i == leftDiffList.count() && j == rightDiffList.count())
                                doAlign = true;
                        if (doAlign) {
                            // align here
                            leftLineAligned = leftLineNumber;
                            rightLineAligned = rightLineNumber;
                            // insert separators if needed
                            if (rightDifference > leftDifference)
                                leftSpans.insert(leftLineNumber, rightDifference - leftDifference);
                            else if (leftDifference > rightDifference)
                                rightSpans.insert(rightLineNumber, leftDifference - rightDifference);
                        }
                    }
                }
                // check if lines are equal
                if ((line < commonLineCount - 1) // before the last common line in equality
                        || (line == commonLineCount - 1 // or the last common line in equality
                            && i == leftDiffList.count() // and it's the last iteration
                            && j == rightDiffList.count())) {
                    if (line > 0 || lastLineEqual)
                        equalLines.insert(leftLineNumber, rightLineNumber);

                if (line > 0)
                    lastLineEqual = true;

                line++;
FileData calculateContextData(const ChunkData &originalData, int contextLinesNumber)
    const int joinChunkThreshold = 1;
    FileData fileData;
            const int firstLine = first ? 0 : equalRowStart + contextLinesNumber;
            const int lastLine = last ? originalData.rows.count() : i - contextLinesNumber;
void addChangedPositions(int positionOffset, const QMap<int, int> &originalChangedPositions, QMap<int, int> *changedPositions)
    QMapIterator<int, int> it(originalChangedPositions);
    while (it.hasNext()) {
        it.next();
        const int startPos = it.key();
        const int endPos = it.value();
        const int newStartPos = startPos < 0 ? -1 : startPos + positionOffset;
        const int newEndPos = endPos < 0 ? -1 : endPos + positionOffset;
        if (startPos < 0 && !changedPositions->isEmpty()) {
           QMap<int, int>::iterator last = changedPositions->end();
           --last;
           last.value() = newEndPos;
        } else
            changedPositions->insert(newStartPos, newEndPos);
QList<QTextEdit::ExtraSelection> colorPositions(
        const QTextCharFormat &format,
        QTextCursor &cursor,
        const QMap<int, int> &positions)
    QList<QTextEdit::ExtraSelection> lineSelections;
    cursor.setPosition(0);
    QMapIterator<int, int> itPositions(positions);
    while (itPositions.hasNext()) {
        itPositions.next();
        cursor.setPosition(itPositions.key());
        cursor.setPosition(itPositions.value(), QTextCursor::KeepAnchor);
        QTextEdit::ExtraSelection selection;
        selection.cursor = cursor;
        selection.format = format;
        lineSelections.append(selection);
    return lineSelections;
QTextCharFormat fullWidthFormatForTextStyle(const TextEditor::FontSettings &fontSettings,
                                            TextEditor::TextStyle textStyle)
    QTextCharFormat format = fontSettings.toTextCharFormat(textStyle);
    format.setProperty(QTextFormat::FullWidthSelection, true);
    return format;
} // namespace Internal