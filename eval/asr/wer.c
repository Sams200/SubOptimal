//
// Created by sams on 7/3/26.
//

#include "wer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static char **tokenize(const char *text,size_t *count){
    char *copy=strdup(text);
    size_t capacity=16;
    size_t n=0;
    char **tokens=malloc(capacity*sizeof(char *));

    char *p=copy;
    char *word;
    while((word=strsep(&p," \t\n\r"))!=NULL){
        if(strlen(word)==0) continue;
        if(n>=capacity){
            capacity*=2;
            tokens=realloc(tokens,capacity*sizeof(char *));
        }
        tokens[n++]=strdup(word);
    }

    free(copy);
    *count=n;
    return tokens;
}

static void free_tokens(char **tokens,size_t count){
    for(size_t i=0;i<count;i++) free(tokens[i]);
    free(tokens);
}

char *normalize_transcript(const char *text){
    size_t len=strlen(text);
    char *norm=malloc(len+1);
    size_t j=0;

    for(size_t i=0;i<len;i++){
        char c=text[i];
        if(isalnum(c)||c=='\''){
            norm[j++]=tolower(c);
        } else if(c=='-'||c=='_'){
            norm[j++]=' ';
        } else if(ispunct(c)){
            continue;
        } else if(isspace(c)){
            norm[j++]=' ';
        }
    }
    norm[j]='\0';

    //collapse multiple spaces
    char *result=malloc(strlen(norm)+1);
    size_t k=0;
    int last_was_space=1;
    for(size_t i=0;i<strlen(norm);i++){
        if(norm[i]==' '){
            if(!last_was_space){
                result[k++]=' ';
                last_was_space=1;
            }
        } else{
            result[k++]=norm[i];
            last_was_space=0;
        }
    }
    if(k>0&&result[k-1]==' ') k--;
    result[k]='\0';
    free(norm);

    return result;
}

double calculate_wer(const char *reference,const char *hypothesis){
    char *ref_norm=normalize_transcript(reference);
    char *hyp_norm=normalize_transcript(hypothesis);

    size_t ref_n,hyp_n;
    char **ref_words=tokenize(ref_norm,&ref_n);
    char **hyp_words=tokenize(hyp_norm,&hyp_n);

    free(ref_norm);
    free(hyp_norm);

    if(ref_n==0){
        free_tokens(hyp_words,hyp_n);
        return (hyp_n==0)?0.0:100.0;
    }

    //levenshtein distance on word level
    size_t rows=ref_n+1;
    size_t cols=hyp_n+1;
    unsigned int **dp=malloc(rows*sizeof(unsigned int *));
    for(size_t i=0;i<rows;i++){
        dp[i]=calloc(cols,sizeof(unsigned int));
        dp[i][0]=i;
    }
    for(size_t j=0;j<cols;j++) dp[0][j]=j;

    for(size_t i=1;i<=ref_n;i++){
        for(size_t j=1;j<=hyp_n;j++){
            unsigned int cost=(strcmp(ref_words[i-1],hyp_words[j-1])==0)?0:1;
            unsigned int del=dp[i-1][j]+1;
            unsigned int ins=dp[i][j-1]+1;
            unsigned int sub=dp[i-1][j-1]+cost;
            dp[i][j]=(del<ins)?((del<sub)?del:sub):((ins<sub)?ins:sub);
        }
    }

    double wer=100.0*dp[ref_n][hyp_n]/(double)ref_n;

    for(size_t i=0;i<rows;i++) free(dp[i]);
    free(dp);
    free_tokens(ref_words,ref_n);
    free_tokens(hyp_words,hyp_n);

    return wer;
}
