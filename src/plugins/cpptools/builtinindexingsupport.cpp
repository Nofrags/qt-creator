#include "builtinindexingsupport.h"

#include "cppmodelmanager.h"
#include "cpppreprocessor.h"
#include "searchsymbols.h"
#include "cpptoolsconstants.h"
#include "cppprojectfile.h"

#include <coreplugin/icore.h>
#include <coreplugin/progressmanager/progressmanager.h>

#include <utils/runextensions.h>

#include <QCoreApplication>

using namespace CppTools;
using namespace CppTools::Internal;

static const bool DumpFileNameWhileParsing = qgetenv("QTC_DUMP_FILENAME_WHILE_PARSING") == "1";

namespace {

static void parse(QFutureInterface<void> &future,
                  CppPreprocessor *preproc,
                  QStringList files)
{
    if (files.isEmpty())
        return;

    QStringList sources;
    QStringList headers;

    foreach (const QString &file, files) {
        preproc->removeFromCache(file);
        if (ProjectFile::isSource(ProjectFile::classify(file)))
            sources.append(file);
        else
            headers.append(file);
    }

    const int sourceCount = sources.size();
    files = sources;
    files += headers;

    preproc->setTodo(files);

    future.setProgressRange(0, files.size());

    const QString conf = CppModelManagerInterface::configurationFileName();
    bool processingHeaders = false;

    CppModelManager *cmm = CppModelManager::instance();
    const QStringList fallbackIncludePaths = cmm->includePaths();
    for (int i = 0; i < files.size(); ++i) {
        if (future.isPaused())
            future.waitForResume();

        if (future.isCanceled())
            break;

        const QString fileName = files.at(i);

        const bool isSourceFile = i < sourceCount;
        if (isSourceFile) {
            (void) preproc->run(conf);
        } else if (!processingHeaders) {
            (void) preproc->run(conf);

            processingHeaders = true;
        }

        QList<ProjectPart::Ptr> parts = cmm->projectPart(fileName);
        QStringList includePaths = parts.isEmpty()
                ? fallbackIncludePaths
                : parts.first()->includePaths;
        preproc->setIncludePaths(includePaths);
        preproc->run(fileName);

        future.setProgressValue(files.size() - preproc->todo().size());

        if (isSourceFile)
            preproc->resetEnvironment();
    }

    future.setProgressValue(files.size());
    preproc->modelManager()->finishedRefreshingSourceFiles(files);

    delete preproc;
}

class BuiltinSymbolSearcher: public SymbolSearcher
{
public:
    BuiltinSymbolSearcher(const CPlusPlus::Snapshot &snapshot,
                          Parameters parameters, QSet<QString> fileNames)
        : m_snapshot(snapshot)
        , m_parameters(parameters)
        , m_fileNames(fileNames)
    {}

    ~BuiltinSymbolSearcher()
    {}

    void runSearch(QFutureInterface<Core::SearchResultItem> &future)
    {
        future.setProgressRange(0, m_snapshot.size());
        future.setProgressValue(0);
        int progress = 0;

        SearchSymbols search;
        search.setSymbolsToSearchFor(m_parameters.types);
        CPlusPlus::Snapshot::const_iterator it = m_snapshot.begin();

        QString findString = (m_parameters.flags & Core::FindRegularExpression
                              ? m_parameters.text : QRegExp::escape(m_parameters.text));
        if (m_parameters.flags & Core::FindWholeWords)
            findString = QString::fromLatin1("\\b%1\\b").arg(findString);
        QRegExp matcher(findString, (m_parameters.flags & Core::FindCaseSensitively
                                     ? Qt::CaseSensitive : Qt::CaseInsensitive));
        while (it != m_snapshot.end()) {
            if (future.isPaused())
                future.waitForResume();
            if (future.isCanceled())
                break;
            if (m_fileNames.isEmpty() || m_fileNames.contains(it.value()->fileName())) {
                QVector<Core::SearchResultItem> resultItems;
                QList<ModelItemInfo> modelInfos = search(it.value());
                foreach (const ModelItemInfo &info, modelInfos) {
                    int index = matcher.indexIn(info.symbolName);
                    if (index != -1) {
                        QString text = info.symbolName;
                        QString scope = info.symbolScope;
                        if (info.type == ModelItemInfo::Method) {
                            QString name;
                            info.unqualifiedNameAndScope(info.symbolName, &name, &scope);
                            text = name + info.symbolType;
                        } else if (info.type == ModelItemInfo::Declaration){
                            text = ModelItemInfo::representDeclaration(info.symbolName,
                                                                       info.symbolType);
                        }

                        Core::SearchResultItem item;
                        item.path = scope.split(QLatin1String("::"), QString::SkipEmptyParts);
                        item.text = text;
                        item.textMarkPos = -1;
                        item.textMarkLength = 0;
                        item.icon = info.icon;
                        item.lineNumber = -1;
                        item.userData = qVariantFromValue(info);
                        resultItems << item;
                    }
                }
                if (!resultItems.isEmpty())
                    future.reportResults(resultItems);
            }
            ++it;
            ++progress;
            future.setProgressValue(progress);
        }
        if (future.isPaused())
            future.waitForResume();
    }

private:
    const CPlusPlus::Snapshot m_snapshot;
    const Parameters m_parameters;
    const QSet<QString> m_fileNames;
};

} // anonymous namespace

BuiltinIndexingSupport::BuiltinIndexingSupport()
    : m_revision(0)
{
    m_synchronizer.setCancelOnWait(true);
    m_dumpFileNameWhileParsing = DumpFileNameWhileParsing;
}

BuiltinIndexingSupport::~BuiltinIndexingSupport()
{}

QFuture<void> BuiltinIndexingSupport::refreshSourceFiles(const QStringList &sourceFiles,
    CppModelManagerInterface::ProgressNotificationMode mode)
{
    CppModelManager *mgr = CppModelManager::instance();
    const WorkingCopy workingCopy = mgr->workingCopy();

    CppPreprocessor *preproc = new CppPreprocessor(mgr, m_dumpFileNameWhileParsing);
    preproc->setRevision(++m_revision);
    preproc->setIncludePaths(mgr->includePaths());
    preproc->setFrameworkPaths(mgr->frameworkPaths());
    preproc->setWorkingCopy(workingCopy);

    QFuture<void> result = QtConcurrent::run(&parse, preproc, sourceFiles);

    if (m_synchronizer.futures().size() > 10) {
        QList<QFuture<void> > futures = m_synchronizer.futures();

        m_synchronizer.clearFutures();

        foreach (const QFuture<void> &future, futures) {
            if (!(future.isFinished() || future.isCanceled()))
                m_synchronizer.addFuture(future);
        }
    }

    m_synchronizer.addFuture(result);

    if (mode == CppModelManagerInterface::ForcedProgressNotification || sourceFiles.count() > 1) {
        Core::ProgressManager::addTask(result, QCoreApplication::translate("CppTools::Internal::BuiltinIndexingSupport", "Parsing"),
                                                CppTools::Constants::TASK_INDEX);
    }

    return result;
}

SymbolSearcher *BuiltinIndexingSupport::createSymbolSearcher(SymbolSearcher::Parameters parameters, QSet<QString> fileNames)
{
    return new BuiltinSymbolSearcher(CppModelManager::instance()->snapshot(), parameters, fileNames);
}
