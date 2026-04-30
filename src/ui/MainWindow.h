#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QThread>
#include "PipelineWorker.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void browseInput();
    void browseOutput();
    void onTranslateToggled(bool checked);
    void onContextCheckToggled(bool checked);
    void startPipeline();
    void cancelPipeline();
    void onPipelineFinished();
    void onPipelineError(const QString &message);
    void onPipelineCancelled();
    void updateProgress();
    void resetState();

private:
    QLineEdit *m_inputEdit;
    QPushButton *m_inputBtn;

    QComboBox *m_modelCombo;

    QCheckBox *m_translateCheck;
    QComboBox *m_languageCombo;

    QCheckBox *m_contextCheck;
    QLineEdit *m_ollamaModelEdit;
    QLineEdit *m_ollamaHostEdit;

    QLineEdit *m_outputEdit;
    QPushButton *m_outputBtn;

    QPushButton *m_startBtn;
    QPushButton *m_cancelBtn;

    QProgressBar *m_transcribeProgress;
    QProgressBar *m_translateProgress;
    QProgressBar *m_contextProgress;

    QThread *m_thread;
    PipelineWorker *m_worker;
    QTimer *m_progressTimer;

    void setControlsEnabled(bool enabled);
};

#endif // MAINWINDOW_H
