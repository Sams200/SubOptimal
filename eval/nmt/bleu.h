//
// Created by sams on 7/3/26.
//

#ifndef BLEU_HPP
#define BLEU_HPP

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <sstream>
#include <algorithm>

class BLEUScorer{
public:
    BLEUScorer(int max_n=4) : max_n_(max_n), ref_len_total_(0), hyp_len_total_(0){
        for(int i=0;i<max_n;++i){
            clipped_counts_.push_back(0);
            total_counts_.push_back(0);
        }
    }

    void addSentence(const std::string &reference,const std::string &hypothesis){
        auto ref_tokens=tokenize(reference);
        auto hyp_tokens=tokenize(hypothesis);

        ref_len_total_+=ref_tokens.size();
        hyp_len_total_+=hyp_tokens.size();

        for(int n=1;n<=max_n_;++n){
            auto ref_ngrams=getNgrams(ref_tokens,n);
            auto hyp_ngrams=getNgrams(hyp_tokens,n);

            int clipped=0;
            for(const auto &[ngram, count]:hyp_ngrams){
                auto it=ref_ngrams.find(ngram);
                if(it!=ref_ngrams.end()){
                    clipped+=std::min(count,it->second);
                }
            }

            clipped_counts_[n-1]+=clipped;
            total_counts_[n-1]+=hyp_ngrams.size();
        }
    }

    double getCorpusBLEU() const{
        if(hyp_len_total_==0) return 0.0;

        double bp=brevityPenalty();

        double log_sum=0.0;
        int valid_n=0;
        for(int i=0;i<max_n_;++i){
            if(total_counts_[i]==0) continue;
            double precision=static_cast<double>(clipped_counts_[i])/total_counts_[i];
            //smoothing to avoid log(0)
            if(precision==0) precision=1e-10;
            log_sum+=std::log(precision);
            valid_n++;
        }

        if(valid_n==0) return 0.0;

        return bp*std::exp(log_sum/valid_n)*100.0;
    }

    void reset(){
        ref_len_total_=0;
        hyp_len_total_=0;
        for(int i=0;i<max_n_;++i){
            clipped_counts_[i]=0;
            total_counts_[i]=0;
        }
    }

private:
    int max_n_;
    std::vector<long long> clipped_counts_;
    std::vector<long long> total_counts_;
    long long ref_len_total_;
    long long hyp_len_total_;

    std::vector<std::string> tokenize(const std::string &text) const{
        std::vector<std::string> tokens;
        std::istringstream iss(text);
        std::string token;
        while(iss>>token){
            std::transform(token.begin(),token.end(),token.begin(),::tolower);
            tokens.push_back(token);
        }
        return tokens;
    }

    std::map<std::vector<std::string>,int> getNgrams(const std::vector<std::string> &tokens,int n) const{
        std::map<std::vector<std::string>,int> counts;
        if(tokens.size()<static_cast<size_t>(n)) return counts;

        for(size_t i=0;i<=tokens.size()-n;++i){
            std::vector<std::string> ngram(tokens.begin()+i,tokens.begin()+i+n);
            counts[ngram]++;
        }
        return counts;
    }

    double brevityPenalty() const{
        if(hyp_len_total_>ref_len_total_) return 1.0;
        return std::exp(1.0-static_cast<double>(ref_len_total_)/hyp_len_total_);
    }
};

#endif // BLEU_HPP
