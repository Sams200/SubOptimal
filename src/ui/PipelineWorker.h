#ifndef PIPELINEWORKER_H
#define PIPELINEWORKER_H

#include <QObject>
#include <QString>

class PipelineWorker : public QObject {
    Q_OBJECT
public:
    explicit PipelineWorker(
        const QString &inputPath,
        const QString &outputPath,
        const QString &model,
        bool translate,
        const QString &targetLang,
        bool contextCheck,
        const QString &ollamaModel,
        const QString &ollamaHost,
        QObject *parent = nullptr);

public slots:
    void process();

signals:
    void finished();
    void pipelineError(const QString &message);
    void cancelled();

private:
    QString m_inputPath;
    QString m_outputPath;
    QString m_model;
    bool m_translate;
    QString m_targetLang;
    bool m_contextCheck;
    QString m_ollamaModel;
    QString m_ollamaHost;
};

#endif // PIPELINEWORKER_H
