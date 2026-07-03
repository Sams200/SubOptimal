//
// Created by sams on 7/3/26.
//

// Stubs for CLI functions
void print_progress(int percent) {
    (void)percent;  // Silent for batch mode
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../src/pipeline/transcribe.h"
#include "wer.h"
#include "../../drwav/dr_wav.h"
#include "../../src/pipeline/loader.h"
#include "../../src/subtitles/subtitles.h"
#include <math.h>

typedef struct{
    double wer;
    char *filename;
    char *reference;
    char *hypothesis;
} result_entry;

static void print_usage(const char *prog){
    fprintf(stderr,"Usage: %s <model.bin> <vad_model.bin> <manifest.tsv> [language]\n",prog);
    fprintf(stderr,"Manifest format: audio_path<tab>reference_text\n");
    fprintf(stderr,"Example: eval/audio.wav\thello world\n");
}

static char *subtitle_list_to_text(subtitle_list *list){
    if(!list||list->count==0) return strdup("");

    size_t total_len=1;
    for(size_t i=0;i<list->count;i++){
        total_len+=strlen(list->segments[i].text)+1; // +1 for space
    }

    char *text=malloc(total_len);
    text[0]='\0';

    for(size_t i=0;i<list->count;i++){
        strcat(text,list->segments[i].text);
        if(i<list->count-1) strcat(text," ");
    }
    return text;
}

int main(int argc,char **argv){
    if(argc<4){
        print_usage(argv[0]);
        return 1;
    }

    const char *model_path=argv[1];
    const char *vad_path=argv[2];
    const char *manifest_path=argv[3];
    const char *language=(argc>4)?argv[4]:NULL;

    FILE *mf=fopen(manifest_path,"r");
    if(!mf){
        fprintf(stderr,"Failed to open manifest: %s\n",manifest_path);
        return 1;
    }

    result_entry *results=NULL;
    size_t num_results=0;
    size_t capacity=100;
    results=malloc(capacity*sizeof(result_entry));

    char line[4096];
    int line_num=0;
    double total_wer=0.0;
    double min_wer=100.0,max_wer=0.0;

    printf("ASR Evaluation\n");
    printf("Model: %s\n",model_path);
    printf("VAD: %s\n",vad_path);
    printf("Language: %s\n",language?language:"auto-detect");
    printf("------------------------\n");

    while(fgets(line,sizeof(line),mf)){
        line_num++;
        line[strcspn(line,"\n")]=0;
        if(strlen(line)==0||line[0]=='#') continue;

        char *tab=strchr(line,'\t');
        if(!tab){
            fprintf(stderr,"Warning: Line %d missing tab delimiter\n",line_num);
            continue;
        }

        *tab='\0';
        const char *audio_path=line;
        const char *reference=tab+1;

        // Strip carriage return if present
        size_t ref_len=strlen(reference);
        if(ref_len>0&&reference[ref_len-1]=='\r'){
            ((char *)reference)[ref_len-1]='\0';
        }

        // Load audio
        float* audio;
        size_t audio_len;
        audio = load_audio(audio_path,&audio_len);

        // Transcribe
        clock_t start=clock();
        subtitle_list *hypothesis_list=transcribe(model_path,audio,
                                                  audio_len,
                                                  vad_path,language);
        clock_t end=clock();
        double duration=(double)(end-start)/CLOCKS_PER_SEC;

        free(audio);

        if(!hypothesis_list){
            fprintf(stderr,"Transcription failed for %s\n",audio_path);
            continue;
        }

        char *hypothesis=subtitle_list_to_text(hypothesis_list);

        double wer=calculate_wer(reference,hypothesis);

        if(num_results>=capacity){
            capacity*=2;
            results=realloc(results,capacity*sizeof(result_entry));
        }

        results[num_results].wer=wer;
        results[num_results].filename=strdup(audio_path);
        results[num_results].reference=strdup(reference);
        results[num_results].hypothesis=hypothesis;
        num_results++;

        total_wer+=wer;
        if(wer<min_wer) min_wer=wer;
        if(wer>max_wer) max_wer=wer;

        printf("WER: %.2f%% (%.2fs)\n",wer,duration);

        free_subtitle_list(hypothesis_list);
    }

    fclose(mf);

    if(num_results==0){
        printf("No valid results\n");
        free(results);
        return 1;
    }

    // Summary
    double mean_wer=total_wer/num_results;

    // Calculate std deviation
    double variance=0;
    for(size_t i=0;i<num_results;i++){
        variance+=pow(results[i].wer-mean_wer,2);
    }
    double stddev=sqrt(variance/num_results);

    printf("\n========== EVALUATION SUMMARY ==========\n");
    printf("Files processed: %zu\n",num_results);
    printf("Mean WER:        %.2f%%\n",mean_wer);
    printf("Std Dev:         %.2f%%\n",stddev);
    printf("Min WER:         %.2f%%\n",min_wer);
    printf("Max WER:         %.2f%%\n",max_wer);
    printf("======================================\n");

    // Cleanup
    for(size_t i=0;i<num_results;i++){
        free(results[i].filename);
        free(results[i].reference);
        free(results[i].hypothesis);
    }
    free(results);

    return 0;
}
