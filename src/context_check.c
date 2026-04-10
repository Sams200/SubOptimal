#include "context_check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include "validator.h"
#include "defaults.h"

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

// json stuff
static char *unescape_json_string(const char *input) {
    if (!input) return strdup("");

    size_t len = strlen(input);
    char *output = malloc(len * 3 + 1);
    if (!output) return strdup("");

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\\' && i + 1 < len) {
            switch (input[i + 1]) {
            case 'n':  output[j++] = '\n'; i++; break;
            case 't':  output[j++] = '\t'; i++; break;
            case 'r':  output[j++] = '\r'; i++; break;
            case '\\': output[j++] = '\\'; i++; break;
            case '"':  output[j++] = '"';  i++; break;
            case '/':  output[j++] = '/';  i++; break;
            case 'u': {
                if (i + 5 < len) {
                    char hex[5];
                    memcpy(hex, input + i + 2, 4);
                    hex[4] = '\0';
                    unsigned int val;
                    if (sscanf(hex, "%x", &val) == 1) {
                        if (val >= 0xD800 && val <= 0xDBFF && i + 11 < len
                            && input[i + 6] == '\\' && input[i + 7] == 'u') {
                            char hex2[5];
                            memcpy(hex2, input + i + 8, 4);
                            hex2[4] = '\0';
                            unsigned int lo;
                            if (sscanf(hex2, "%x", &lo) == 1
                                && lo >= 0xDC00 && lo <= 0xDFFF) {
                                unsigned int cp = 0x10000
                                    + ((val - 0xD800) << 10)
                                    + (lo - 0xDC00);
                                output[j++] = 0xF0 | (cp >> 18);
                                output[j++] = 0x80 | ((cp >> 12) & 0x3F);
                                output[j++] = 0x80 | ((cp >> 6) & 0x3F);
                                output[j++] = 0x80 | (cp & 0x3F);
                                i += 11;
                                break;
                            }
                        }
                        if (val < 0x80) {
                            output[j++] = (char)val;
                        } else if (val < 0x800) {
                            output[j++] = 0xC0 | (val >> 6);
                            output[j++] = 0x80 | (val & 0x3F);
                        } else {
                            output[j++] = 0xE0 | (val >> 12);
                            output[j++] = 0x80 | ((val >> 6) & 0x3F);
                            output[j++] = 0x80 | (val & 0x3F);
                        }
                    }
                    i += 5;
                } else {
                    output[j++] = input[i];
                }
                break;
            }
            default:
                output[j++] = input[i];
                break;
            }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
    return output;
}

static char *escape_json_string(const char *input) {
    if (!input) return strdup("");

    size_t len = strlen(input);
    char *output = malloc(len * 6 + 1);
    if (!output) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)input[i];
        switch (c) {
        case '"':  output[j++] = '\\'; output[j++] = '"';  break;
        case '\\': output[j++] = '\\'; output[j++] = '\\'; break;
        case '\n': output[j++] = '\\'; output[j++] = 'n';  break;
        case '\r': output[j++] = '\\'; output[j++] = 'r';  break;
        case '\t': output[j++] = '\\'; output[j++] = 't';  break;
        case '\b': output[j++] = '\\'; output[j++] = 'b';  break;
        case '\f': output[j++] = '\\'; output[j++] = 'f';  break;
        default:
            if (c < 0x20) {
                j += sprintf(output + j, "\\u%04x", c);
            } else {
                output[j++] = c;
            }
            break;
        }
    }
    output[j] = '\0';
    return output;
}

static char *extract_response_field(const char *json) {
    if (!json) return NULL;

    const char *key = "\"response\":\"";
    char *start = strstr(json, key);
    if (!start) return NULL;
    start += strlen(key);

    char *end = start;
    while (*end) {
        if (*end == '\\') {
            end += 2;
            continue;
        }
        if (*end == '"') break;
        end++;
    }
    if (*end != '"') return NULL;

    size_t len = end - start;
    char *raw = malloc(len + 1);
    if (!raw) return NULL;
    memcpy(raw, start, len);
    raw[len] = '\0';

    char *unescaped = unescape_json_string(raw);
    free(raw);
    return unescaped;
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
    if (!original && !translated) return;
    if (!curl || !ollama_host) {
        fprintf(stderr, "context_check: Not initialized\n");
        return;
    }

    printf("Validating subtitles...\n");
    fflush(stdout);

    subtitle_list *active = translated ? translated : original;
    const char *src = original ? original->language : "unknown";
    const char *tgt = translated ? translated->language : src;

    int batch_size = 40;
    int overlap = 4;

    int total = (int)active->count;
    if (translated && original) {
        int min = (int)original->count < (int)translated->count
                ? (int)original->count : (int)translated->count;
        total = min;
    }

    for (int start_idx = 0; start_idx < total; start_idx += (batch_size)) {
        int end_idx = start_idx + batch_size;
        if (end_idx > total) end_idx = total;
        if (start_idx >= end_idx) break;

        int context_start = start_idx - overlap;
        if (context_start < 0) context_start = 0;

        // Display progress
        int batch_number = start_idx/batch_size + 1;
        int total_batch = total/batch_size + 1;
        printf("\rBatch %d/%d: in progress",batch_number, total_batch);
        fflush(stdout);

        // build prompt
        char *prompt = malloc(MAX_PROMPT_LEN);
        if (!prompt) continue;

        int offset = snprintf(prompt, MAX_PROMPT_LEN, system_prompt, src, tgt);
        offset += snprintf(prompt + offset, MAX_PROMPT_LEN - offset, "\n\n");

        for (int i = context_start; i < end_idx && offset < MAX_PROMPT_LEN - 1024; i++) {
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
        char *model_esc = escape_json_string(model);
        char *prompt_esc = escape_json_string(prompt);
        free(prompt);

        if (!model_esc || !prompt_esc) {
            free(model_esc);
            free(prompt_esc);
            fprintf(stderr, "context_check: Failed to escape JSON strings\n");
            continue;
        }

        size_t req_len = strlen(model_esc) + strlen(prompt_esc) + 128;
        char *request = malloc(req_len);
        if (!request) {
            free(model_esc);
            free(prompt_esc);
            continue;
        }

        snprintf(request, req_len,
            "{\"model\":\"%s\",\"prompt\":\"%s\",\"stream\":false,\"think\":false}",
            model_esc, prompt_esc);

        free(model_esc);
        free(prompt_esc);

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
        char *srt_text = extract_response_field(resp.data);
        free(resp.data);

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
            batch_number, total_batch, applied, parsed->count);
        fflush(stdout);

        free_subtitle_list(parsed);
    }
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