#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include "defaults.h"
#include "cancel.h"
#include "transcribe.h"
#include "translate.h"
#include "context_check.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    auto *formLayout = new QFormLayout();
    mainLayout->addLayout(formLayout);

    // Input file
    auto *inputLayout = new QHBoxLayout();
    m_inputEdit = new QLineEdit(this);
    m_inputBtn = new QPushButton("Browse", this);
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_inputBtn);
    formLayout->addRow("Input Video:", inputLayout);

    // Model dropdown
    m_modelCombo = new QComboBox(this);
    for (int i = 0; !is_end_of_array(TRANSCRIBE_MODEL_NAMES[i]); i++) {
        m_modelCombo->addItem(TRANSCRIBE_MODEL_NAMES[i], i);
    }
    m_modelCombo->setCurrentIndex(3); // base.en
    formLayout->addRow("Model:", m_modelCombo);

    // Translation
    m_translateCheck = new QCheckBox("Enable Translation", this);
    m_languageCombo = new QComboBox(this);
    for (int i = 0; !is_end_of_array(VALID_LANGUAGES[i]); i++) {
        m_languageCombo->addItem(LANGUAGE_NAMES[i], QString(VALID_LANGUAGES[i]));
    }
    m_languageCombo->setEditable(false);
    m_languageCombo->hide();
    auto *transLayout = new QHBoxLayout();
    transLayout->addWidget(m_translateCheck);
    transLayout->addWidget(m_languageCombo, 1);
    formLayout->addRow("Translation:", transLayout);

    // Context check
    m_contextCheck = new QCheckBox("Enable Context Check", this);
    m_ollamaModelEdit = new QLineEdit(this);
    m_ollamaModelEdit->setPlaceholderText("mistral");
    m_ollamaModelEdit->hide();
    auto *ctxLayout = new QHBoxLayout();
    ctxLayout->addWidget(m_contextCheck);
    ctxLayout->addWidget(m_ollamaModelEdit, 1);
    formLayout->addRow("AI Check:", ctxLayout);

    // Ollama host
    m_ollamaHostEdit = new QLineEdit(this);
    m_ollamaHostEdit->setText("http://localhost:11434");
    formLayout->addRow("Ollama Host:", m_ollamaHostEdit);

    // Output file
    auto *outputLayout = new QHBoxLayout();
    m_outputEdit = new QLineEdit(this);
    m_outputEdit->setText("output.srt");
    m_outputBtn = new QPushButton("Browse", this);
    outputLayout->addWidget(m_outputEdit);
    outputLayout->addWidget(m_outputBtn);
    formLayout->addRow("Output SRT:", outputLayout);

    // Progress bars
    auto *progressGroup = new QGroupBox("Progress", this);
    auto *progressLayout = new QVBoxLayout(progressGroup);
    m_transcribeProgress = new QProgressBar(this);
    m_transcribeProgress->setRange(0, 100);
    m_transcribeProgress->setValue(0);
    m_translateProgress = new QProgressBar(this);
    m_translateProgress->setRange(0, 100);
    m_translateProgress->setValue(0);
    m_contextProgress = new QProgressBar(this);
    m_contextProgress->setRange(0, 100);
    m_contextProgress->setValue(0);
    progressLayout->addWidget(m_transcribeProgress);
    progressLayout->addWidget(m_translateProgress);
    progressLayout->addWidget(m_contextProgress);
    mainLayout->addWidget(progressGroup);

    // Buttons
    auto *btnLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("Start", this);
    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setEnabled(false);
    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    m_progressTimer = new QTimer(this);
    m_progressTimer->setInterval(200);

    connect(m_inputBtn, &QPushButton::clicked, this, &MainWindow::browseInput);
    connect(m_outputBtn, &QPushButton::clicked, this, &MainWindow::browseOutput);
    connect(m_translateCheck, &QCheckBox::toggled, this, &MainWindow::onTranslateToggled);
    connect(m_contextCheck, &QCheckBox::toggled, this, &MainWindow::onContextCheckToggled);
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::startPipeline);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::cancelPipeline);
    connect(m_progressTimer, &QTimer::timeout, this, &MainWindow::updateProgress);

    setWindowTitle("SubOptimal - Subtitle Generator");
    resize(600, 400);
}

void MainWindow::browseInput() {
    QString path = QFileDialog::getOpenFileName(this, "Select Input Video", QString(),
        "Video Files (*.mp4 *.avi *.mkv *.mov *.webm *.flv);;All Files (*)");
    if (!path.isEmpty()) m_inputEdit->setText(path);
}

void MainWindow::browseOutput() {
    QString path = QFileDialog::getSaveFileName(this, "Save SRT Output", ".srt",
        "Subtitle Files (*.srt);;All Files (*)");
    if (!path.isEmpty()) m_outputEdit->setText(path);
}

void MainWindow::onTranslateToggled(bool checked) {
    if (checked) m_languageCombo->show();
    else m_languageCombo->hide();
    adjustSize();
}

void MainWindow::onContextCheckToggled(bool checked) {
    if (checked) m_ollamaModelEdit->show();
    else m_ollamaModelEdit->hide();
    adjustSize();
}

void MainWindow::setControlsEnabled(bool enabled) {
    m_inputEdit->setEnabled(enabled);
    m_inputBtn->setEnabled(enabled);
    m_modelCombo->setEnabled(enabled);
    m_translateCheck->setEnabled(enabled);
    m_languageCombo->setEnabled(enabled);
    m_contextCheck->setEnabled(enabled);
    m_ollamaModelEdit->setEnabled(enabled);
    m_ollamaHostEdit->setEnabled(enabled);
    m_outputEdit->setEnabled(enabled);
    m_outputBtn->setEnabled(enabled);
}

void MainWindow::startPipeline() {
    QString inputPath = m_inputEdit->text().trimmed();
    QString outputPath = m_outputEdit->text().trimmed();

    if (inputPath.isEmpty() || outputPath.isEmpty()) {
        QMessageBox::warning(this, "Missing Fields", "Please specify input and output paths.");
        return;
    }

    int modelIndex = m_modelCombo->currentData().toInt();
    const char *modelFull = TRANSCRIBE_MODEL_NAMES_FULL[modelIndex];

    bool translate = m_translateCheck->isChecked();
    QString targetLang = translate ? m_languageCombo->currentData().toString() : QString();

    bool contextCheck = m_contextCheck->isChecked();
    QString ollamaModel = contextCheck ? m_ollamaModelEdit->text().trimmed() : QString();
    if (contextCheck && ollamaModel.isEmpty()) ollamaModel = "mistral";
    QString ollamaHost = m_ollamaHostEdit->text().trimmed();

    resetState();
    setControlsEnabled(false);
    m_startBtn->setEnabled(false);
    m_cancelBtn->setEnabled(true);

    m_thread = new QThread(this);
    m_worker = new PipelineWorker(inputPath, outputPath, QString(modelFull),
                                  translate, targetLang, contextCheck, ollamaModel,
                                  ollamaHost);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &PipelineWorker::process);
    connect(m_worker, &PipelineWorker::finished, this, &MainWindow::onPipelineFinished);
    connect(m_worker, &PipelineWorker::pipelineError, this, &MainWindow::onPipelineError);
    connect(m_worker, &PipelineWorker::cancelled, this, &MainWindow::onPipelineCancelled);
    connect(m_worker, &PipelineWorker::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &PipelineWorker::cancelled, m_worker, &QObject::deleteLater);
    connect(m_worker, &PipelineWorker::pipelineError, m_worker, &QObject::deleteLater);
    connect(m_worker, &PipelineWorker::finished, m_thread, &QThread::quit);
    connect(m_worker, &PipelineWorker::cancelled, m_thread, &QThread::quit);
    connect(m_worker, &PipelineWorker::pipelineError, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    m_thread->start();
    m_progressTimer->start();
}

void MainWindow::cancelPipeline() {
    cancel_signal();
    m_cancelBtn->setEnabled(false);
}

void MainWindow::onPipelineFinished() {
    m_progressTimer->stop();
    updateProgress();
    m_startBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    setControlsEnabled(true);
    QMessageBox::information(this, "Done", "Pipeline completed successfully.");
}

void MainWindow::onPipelineError(const QString &message) {
    m_progressTimer->stop();
    m_startBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    setControlsEnabled(true);
    QMessageBox::critical(this, "Error", message);
    fprintf(stderr, "GUI Pipeline Error: %s\n", message.toUtf8().constData());
}

void MainWindow::onPipelineCancelled() {
    m_progressTimer->stop();
    m_startBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    setControlsEnabled(true);
    QMessageBox::information(this, "Cancelled", "Pipeline was cancelled.");
}

void MainWindow::updateProgress() {
    m_transcribeProgress->setValue(get_transcribe_progress_percent());
    m_translateProgress->setValue(get_translate_progress_percent());
    m_contextProgress->setValue(get_context_progress_percent());
}

void MainWindow::resetState() {
    m_transcribeProgress->setValue(0);
    m_translateProgress->setValue(0);
    m_contextProgress->setValue(0);
    cancel_reset();
}
