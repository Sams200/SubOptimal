//
// Created by sams on 7/3/26.
//

#include "evaluate_nmt.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <iomanip>

#include "../../src/pipeline/translate.h"
#include "../../src/subtitles/subtitles.h"
#include "bleu.h"

// Stub for CLI
extern "C" void print_progress(int percent) {
    (void)percent;
}

struct EvalEntry {
    std::string source;
    std::string reference;
    std::string hypothesis;
    double sentence_bleu;
};

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <model_dir> <spm_model> <manifest.tsv> [max_lines]\n";
    std::cerr << "Manifest format: SourcePath\\tRefPath\\tSrcLang\\tTgtLang\n";
    std::cerr << "Example: eval/nmt/flores_manifest.tsv\n";
}

static subtitle_list* create_subtitle_list(const std::vector<std::string>& lines) {
    subtitle_list* list = (subtitle_list*)malloc(sizeof(subtitle_list));
    if (!list) return nullptr;

    list->count = lines.size();
    list->capacity = lines.size();
    list->language = nullptr;
    list->segments = (subtitle_segment*)calloc(lines.size(), sizeof(subtitle_segment));

    for (size_t i = 0; i < lines.size(); ++i) {
        list->segments[i].id = i + 1;
        strncpy(list->segments[i].t0, "00:00:00,000", sizeof(list->segments[i].t0));
        strncpy(list->segments[i].t1, "00:00:05,000", sizeof(list->segments[i].t1));
        list->segments[i].text = strdup(lines[i].c_str());
    }

    return list;
}

static std::vector<std::string> read_lines(const std::string& path, size_t max_lines = 0) {
    std::vector<std::string> lines;
    std::ifstream file(path);
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') {
            lines.push_back(line);
            if (max_lines > 0 && lines.size() >= max_lines) break;
        }
    }

    return lines;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    const char* model_dir = argv[1];
    const char* spm_path = argv[2];
    const char* manifest_path = argv[3];
    size_t max_lines = (argc > 4) ? std::stoull(argv[4]) : 0;

    std::ifstream mf(manifest_path);
    if (!mf.is_open()) {
        std::cerr << "Failed to open manifest: " << manifest_path << "\n";
        return 1;
    }

    std::string src_path, ref_path, src_lang, tgt_lang;
    std::string line;
    bool found_header = false;

    while (std::getline(mf, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (!found_header) {
            found_header = true;
            continue;
        }

        std::istringstream iss(line);
        std::getline(iss, src_path, '\t');
        std::getline(iss, ref_path, '\t');
        std::getline(iss, src_lang, '\t');
        std::getline(iss, tgt_lang, '\t');
        break;
    }

    if (src_path.empty()) {
        std::cerr << "No valid entry found in manifest\n";
        return 1;
    }

    std::cout << "NMT Evaluation\n";
    std::cout << "Model: " << model_dir << "\n";
    std::cout << "Source: " << src_lang << " (" << src_path << ")\n";
    std::cout << "Target: " << tgt_lang << " (" << ref_path << ")\n";
    std::cout << "------------------------\n";

    // Load data
    auto sources = read_lines(src_path, max_lines);
    auto references = read_lines(ref_path, max_lines);

    if (sources.empty() || references.empty()) {
        std::cerr << "Failed to load source or reference files\n";
        return 1;
    }

    if (sources.size() != references.size()) {
        std::cerr << "Mismatched line counts: " << sources.size() << " vs " << references.size() << "\n";
        return 1;
    }

    std::cout << "Loaded " << sources.size() << " sentence pairs\n\n";

    try {
        translator_init(model_dir, spm_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize translator: " << e.what() << "\n";
        return 1;
    }

    BLEUScorer bleu;
    std::vector<EvalEntry> results;
    results.reserve(sources.size());

    const size_t batch_size = 50;
    auto start_total = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < sources.size(); i += batch_size) {
        size_t end = std::min(i + batch_size, sources.size());

        std::vector<std::string> batch_src(sources.begin() + i, sources.begin() + end);
        subtitle_list* sub_list = create_subtitle_list(batch_src);
        if (!sub_list) {
            std::cerr << "Memory allocation failed\n";
            translator_free();
            return 1;
        }

        translate_subtitles(sub_list, src_lang.c_str(), tgt_lang.c_str());

        for (size_t j = 0; j < sub_list->count; ++j) {
            size_t idx = i + j;
            std::string hyp = sub_list->segments[j].text ? sub_list->segments[j].text : "";

            BLEUScorer sent_bleu;
            sent_bleu.addSentence(references[idx], hyp);

            EvalEntry entry;
            entry.source = sources[idx];
            entry.reference = references[idx];
            entry.hypothesis = hyp;
            entry.sentence_bleu = sent_bleu.getCorpusBLEU();
            results.push_back(entry);

            bleu.addSentence(references[idx], hyp);
        }

        free_subtitle_list(sub_list);

        int progress = (end * 100) / sources.size();
        if (i % 100 == 0 || end == sources.size()) {
            std::cout << "Progress: " << end << "/" << sources.size()
                     << " (" << progress << "%) - Current Corpus BLEU: "
                     << std::fixed << std::setprecision(2) << bleu.getCorpusBLEU() << "\n";
        }
    }

    auto end_total = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end_total - start_total).count();

    translator_free();

    double corpus_bleu = bleu.getCorpusBLEU();

    double avg_sent_bleu = 0.0;
    for (const auto& r : results) avg_sent_bleu += r.sentence_bleu;
    avg_sent_bleu /= results.size();

    std::cout << "\n========== EVALUATION SUMMARY ==========\n";
    std::cout << "Sentences processed: " << results.size() << "\n";
    std::cout << "Corpus BLEU:         " << std::fixed << std::setprecision(2) << corpus_bleu << "\n";
    std::cout << "Avg Sentence BLEU:   " << std::fixed << std::setprecision(2) << avg_sent_bleu << "\n";
    std::cout << "Total time:          " << std::fixed << std::setprecision(2) << duration << "s\n";
    std::cout << "Sentences/sec:       " << std::fixed << std::setprecision(2) << results.size()/duration << "\n";
    std::cout << "======================================\n";

    return 0;
}