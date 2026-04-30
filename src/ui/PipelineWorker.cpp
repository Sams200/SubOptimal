#include "PipelineWorker.h"
#include <string>
#include "cli.h"
#include "cancel.h"
#include "transcribe.h"
#include "translate.h"
#include "context_check.h"
#include "subtitles.h"

PipelineWorker::PipelineWorker(
    const QString &inputPath,
    const QString &outputPath,
    const QString &model,
    bool translate,
    const QString &targetLang,
    bool contextCheck,
    const QString &ollamaModel,
    const QString &ollamaHost,
    QObject *parent)
    : QObject(parent)
    , m_inputPath(inputPath)
    , m_outputPath(outputPath)
    , m_model(model)
    , m_translate(translate)
    , m_targetLang(targetLang)
    , m_contextCheck(contextCheck)
    , m_ollamaModel(ollamaModel)
    , m_ollamaHost(ollamaHost)
{
}

void PipelineWorker::process() {
    int error = 0;

    std::string input = m_inputPath.toStdString();
    std::string output = m_outputPath.toStdString();
    std::string model = m_model.toStdString();
    std::string targetLang = m_targetLang.toStdString();
    std::string ollamaModel = m_ollamaModel.toStdString();
    std::string ollamaHost = m_ollamaHost.toStdString();

    subtitle_list *original = perform_transcribe(model.c_str(), input.c_str(), nullptr, &error);

    if (is_cancelled()) {
        if (original) free_subtitle_list(original);
        emit cancelled();
        return;
    }

    if (error || !original) {
        if (original) free_subtitle_list(original);
        emit pipelineError("Transcription failed.");
        return;
    }

    subtitle_list *translated = nullptr;
    if (m_translate) {
        translated = duplicate_list(original);
        perform_translate(translated, targetLang.c_str(), &error);

        if (is_cancelled()) {
            free_subtitle_list(translated);
            free_subtitle_list(original);
            emit cancelled();
            return;
        }

        if (error) {
            free_subtitle_list(translated);
            free_subtitle_list(original);
            emit pipelineError("Translation failed.");
            return;
        }
    }

    if (m_contextCheck) {
        context_check_init(ollamaHost.c_str());

        if (translated) {
            context_check_subtitles(original, translated, ollamaModel.c_str());
        } else {
            context_check_subtitles(original, NULL, ollamaModel.c_str());
        }

        context_check_free();
    }

    if (is_cancelled()) {
        if (translated) free_subtitle_list(translated);
        free_subtitle_list(original);
        emit cancelled();
        return;
    }

    subtitle_list *out_list = translated ? translated : original;
    int write_err = write_subtitles_to_file(output.c_str(), out_list);

    if (write_err != 0) {
        if (translated) free_subtitle_list(translated);
        free_subtitle_list(original);
        emit pipelineError("Failed to write output file.");
        return;
    }

    if (translated) free_subtitle_list(translated);
    free_subtitle_list(original);
    emit finished();
}
