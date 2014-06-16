    toolsContainer->insertGroup(Core::Constants::G_TOOLS_OPTIONS, Constants::G_TOOLS_DIFF);
    const Core::Id editorId = Constants::DIFF_EDITOR_ID;
    //: Editor title
    QString title = tr("Diff \"%1\", \"%2\"").arg(fileName1).arg(fileName2);
    DiffEditor *editor = qobject_cast<DiffEditor *>
            (Core::EditorManager::openEditorWithContents(editorId, &title, QByteArray(),
                                                         (Core::EditorManager::OpenInOtherSplit
                                                          | Core::EditorManager::NoNewSplits)));
    if (!editor)
        return;
    const QString text1 = getFileContents(fileName1);
    const QString text2 = getFileContents(fileName2);
    DiffEditorController::DiffFilesContents dfc;
    dfc.leftFileInfo = fileName1;
    dfc.leftText = text1;
    dfc.rightFileInfo = fileName2;
    dfc.rightText = text2;
    QList<DiffEditorController::DiffFilesContents> list;
    list.append(dfc);
    editor->controller()->setDiffContents(list);
QString DiffEditorPlugin::getFileContents(const QString &fileName) const
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        return Core::EditorManager::defaultTextCodec()->toUnicode(file.readAll());
    return QString();
} // namespace Internal
} // namespace DiffEditor