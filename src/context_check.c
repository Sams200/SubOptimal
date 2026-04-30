#include "context_check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdatomic.h>
#include <curl/curl.h>
#include "validator.h"
#include "defaults.h"
#include "cJSON.h"
#include "cancel.h"

#define MAX_PROMPT_LEN (1 << 18)  /* 256KB */

static CURL *curl = NULL;
static char *ollama_host = NULL;

static const char *system_prompt =
    "You are an expert subtitle validator and translator.\n\n"
    "Your task:\n"
    "1. Review the transcribed and translated subtitle for accuracy and natural phrasing\n"
    "2. Output ONLY the corrected subtitles in valid SRT format\n"
    "3. Fix translation errors (grammar, context, meaning)\n"
    "4. Keep the same target language as the translation\n"
    "5. If translation is NULL, keep the same target language as the original\n"
    "6. Use the Original timestamp and Original text as context for the final timestamp and text\n"
    "7. If the grammar is correct, do NOT rephrase the subtitles\n"
    "8. If you find blatant repetition or transcription errors, you may remove the relevant text or subtitle entirely\n"
    "9. If you want to remove a subtitle completely, replace the text with \"*\"\n\n"
    "INPUT FORMAT:\n"
    "Segment ID: <number>\n"
    "Original timestamp: <HH:MM:SS,mmm --> HH:MM:SS,mmm>\n"
    "Original text: <original subtitle text>\n"
    "Translation: <translated subtitle text>\n\n"
    "Metadata:\n"
    "Source language: %s\n"
    "Target language: %s\n\n"
    "OUTPUT FORMAT:\n"
    "<number>\n"
    "<HH:MM:SS,mmm --> HH:MM:SS,mmm>\n"
    "<validated subtitle text>\n\n"
    "RULES:\n"
    "- If translation is accurate, return it unchanged\n"
    "- If translation needs correction, return ONLY the corrected subtitle, WITHOUT highlights\n"
    "- Maintain SRT format: ID, timestamps, text\n"
    "- Preserve EXACTLY the original timestamps\n"
    "- Ensure output is in the target language\n"
    "- Separate each subtitle block with a blank line\n\n"
    "Return ONLY the corrected SRT blocks, no explanations.\n";

static atomic_int context_progress_percent = 0;
int get_context_progress_percent(){
    return atomic_load(&context_progress_percent);
}
void set_context_progress_percent(int percent){
    atomic_store(&context_progress_percent, percent);
}

// curl stuff
typedef struct {
    char *data;
    size_t size;
} response_buffer;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    response_buffer *buf = userdata;

    char *tmp = realloc(buf->data, buf->size+total+1);
    if (!tmp) return 0;

    buf->data = tmp;
    memcpy(buf->data + buf->size, contents, total);
    buf->size += total;
    buf->data[buf->size] = '\0';

    return total;
}

int context_check_init(const char *host) {
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "context_check: Failed to initialize curl\n");
        return -1;
    }

    ollama_host = strdup(host);
    if (!ollama_host) {
        fprintf(stderr, "context_check: Failed to allocate host string\n");
        curl_easy_cleanup(curl);
        curl = NULL;
        return -1;
    }

    return 0;
}

void context_check_subtitles(
    subtitle_list *original,
    subtitle_list *translated,
    const char *model)
{
    atomic_store(&context_progress_percent, 0);

    if (!original && !translated) return;
    if (!curl || !ollama_host) {
        fprintf(stderr, "context_check: Not initialized\n");
        return;
    }

    printf("Validating subtitles:\n");
    fflush(stdout);

    subtitle_list *active = translated ? translated : original;
    const char *src = original ? original->language : "unknown";
    const char *tgt = translated ? translated->language : src;

    // get full language name
    int lang_idx=0;
    while(!is_end_of_array(WHISPER_LANGUAGE_CODES[lang_idx])) {
        if(strncmp_safe(WHISPER_LANGUAGE_CODES[lang_idx],src,3)) {
            src = WHISPER_LANGUAGE_NAMES[lang_idx]? WHISPER_LANGUAGE_NAMES[lang_idx]: src;
            break;
        }
        lang_idx++;
    }

    int batch_size = 40;
    int overlap = 4;

    int total = (int)active->count;
    if (translated && original) {
        int min = (int)original->count < (int)translated->count
                ? (int)original->count : (int)translated->count;
        total = min;
    }

    for (int start_idx=0; start_idx<total; start_idx+=batch_size) {
        if (is_cancelled()) {
            fprintf(stderr, "Context check cancelled by user\n");
            atomic_store(&context_progress_percent, 0);
            return;
        }

        int end_idx = start_idx + batch_size;
        if (end_idx > total) end_idx = total;
        if (start_idx >= end_idx) break;

        int context_start = start_idx - overlap;
        if (context_start < 0) context_start = 0;

        // Display progress
        int batch_number = start_idx/batch_size + 1;
        int total_batch = total/batch_size + 1;
        printf("\rBatch %d/%d: in progress", batch_number, total_batch);
        fflush(stdout);

        // build prompt
        char *prompt = malloc(MAX_PROMPT_LEN);
        if (!prompt) continue;

        int offset = snprintf(prompt, MAX_PROMPT_LEN, system_prompt, src, tgt);
        offset += snprintf(prompt + offset, MAX_PROMPT_LEN - offset, "\n\n");

        for (int i=context_start; i<end_idx && offset<MAX_PROMPT_LEN-1024; i++) {
            subtitle_segment *seg = &active->segments[i];
            char *t0 = seg->t0;
            char *t1 = seg->t1;

            const char *orig_text = "NULL";
            if (original && i < (int)original->count)
                orig_text = original->segments[i].text ? original->segments[i].text : "NULL";

            const char *trans_text = "NULL";
            if (translated && i < (int)translated->count)
                trans_text = translated->segments[i].text ? translated->segments[i].text : "NULL";

            offset += snprintf(prompt + offset, MAX_PROMPT_LEN - offset,
                "Segment %d:\n"
                "  Original timestamp: %s --> %s\n"
                "  Original text: %s\n"
                "  Translation: %s\n\n",
                i + 1, t0, t1, orig_text, trans_text);
        }

        // build json
        cJSON *req_json = cJSON_CreateObject();
        cJSON_AddStringToObject(req_json, "model", model);
        cJSON_AddStringToObject(req_json, "prompt", prompt);
        cJSON_AddBoolToObject(req_json, "stream", 0);
        cJSON_AddBoolToObject(req_json, "think", 0);
        char *request = cJSON_PrintUnformatted(req_json);
        cJSON_Delete(req_json);
        free(prompt);

        // build url
        size_t url_len = strlen(ollama_host) + 32;
        char *url = malloc(url_len);
        if (!url) { free(request); continue; }
        snprintf(url, url_len, "%s/api/generate", ollama_host);

        // make req
        response_buffer resp = { .data = NULL, .size = 0 };

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(request));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);
        free(url);
        free(request);

        if (res != CURLE_OK) {
            fprintf(stderr, "context_check: curl error: %s\n", curl_easy_strerror(res));
            free(resp.data);
            continue;
        }

        // parse res
        cJSON *resp_json = cJSON_Parse(resp.data);
        free(resp.data);
        char *srt_text = NULL;
        if (resp_json) {
            cJSON *resp_field = cJSON_GetObjectItemCaseSensitive(resp_json, "response");
            if (cJSON_IsString(resp_field) && resp_field->valuestring)
                srt_text = strdup(resp_field->valuestring);
            cJSON_Delete(resp_json);
        }

        if (!srt_text) {
            fprintf(stderr, "context_check: Failed to extract response from JSON\n");
            continue;
        }

        subtitle_list *parsed = parse_srt_response(srt_text);
        free(srt_text);

        if (!parsed || parsed->count == 0) {
            fprintf(stderr, "context_check: No valid SRT entries parsed from AI response\n");
            free_subtitle_list(parsed);
            continue;
        }

        int applied = 0;
        for (int i = start_idx; i < end_idx; i++) {
            subtitle_segment *seg = &active->segments[i];

            subtitle_segment *match = NULL;
            for (size_t e = 0; e < parsed->count; e++) {
                subtitle_segment *entry = &parsed->segments[e];
                if (strcmp(seg->t0, entry->t0) == 0 &&
                    strcmp(seg->t1, entry->t1) == 0 &&
                    entry->text && entry->text[0] != '\0') {
                    match = entry;
                    break;
                }
            }

            if (!match) {
                continue;
            }

            free(seg->text);
            seg->text = strdup(match->text);
            if (!seg->text) seg->text = strdup("");
            applied++;
        }

        printf("\rBatch %d/%d: applied %d/%zu corrections\n",
            batch_number, total_batch, applied, start_idx==0 ? parsed->count : parsed->count-overlap);
        fflush(stdout);
        atomic_store(&context_progress_percent, (batch_number)*100/total_batch);

        free_subtitle_list(parsed);
    }
    atomic_store(&context_progress_percent, 100);
}

void context_check_free(void) {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = NULL;
    }
    if (ollama_host) {
        free(ollama_host);
        ollama_host = NULL;
    }
}