//
// Created by 赵先生 on 2021/7/5.
//

#ifndef CODE_BRANCHPREDICTOR_HPP
#define CODE_BRANCHPREDICTOR_HPP
class twoBits{ // 2-bit prediction scheme
public:
    bool first,second;
    int correct,wrong;
    unsigned int modifiedPc;
public:
    twoBits():first(true),second(false),correct(0),wrong(0),modifiedPc(0u){}
    void modify(bool flag){
        if(first && second){
            if(flag){
                ++correct;
                return;
            }else{
                ++wrong;
                second = false;
                return;
            }
        }
        if((!first) && (!second)){
            if(!flag){
                ++correct;
                return;
            }else {
                second = true;
                ++wrong;
                return;
            }
        }
        if(first && (!second)){
            if(flag){
                ++correct;
                second = true;
                return;
            }else{
                first = false;
                ++wrong;
                return;
            }
        }
        if((!first) && second){
            if(!flag){
                ++correct;
                second = false;
                return;
            }else{
                ++wrong;
                first = true;
            }
        }
    }
    bool predict() const{
        return first;
    }
};

class branchPredictor{
private:
    twoBits table[4096][64]; // 6-bits history record
    unsigned int historyRecord[4096] = {0};
    const unsigned int lowbits1;
    const unsigned int lowbits2;
private:
    unsigned int hash1(unsigned int pc) const {
        return pc & lowbits1;
    }
    unsigned int hash2(unsigned int pc) const{
        return historyRecord[hash1(pc)] & lowbits2;
    }
public:
    branchPredictor():lowbits1(0b111111111111u),lowbits2(0b111111u){};

    bool predict(unsigned int pc) const{
        return table[hash1(pc)][hash2(pc)].predict();
    }

    void modify(unsigned int pc,bool flag){
        table[hash1(pc)][hash2(pc)].modify(flag);
        historyRecord[hash1(pc)] <<= 1;
        if(flag) historyRecord[hash1(pc)] += 1;
    }

    double efficiency() const{
        unsigned int correct = 0,wrong = 0;
        for(int i = 0;i < 4096;++i){
            for(int j = 0;j < 64;++j) {
                correct += table[i][j].correct;
                wrong += table[i][j].wrong;
            }
        }
        return (double) correct / (correct + wrong);
    }

    int times() const{
        int ans = 0;
        for(int i = 0;i < 4096;++i){
            for(int j = 0;j < 64;++j) {
                ans += table[i][j].correct;
                ans += table[i][j].wrong;
            }
        }
        return ans;
    }

    void setModifyPc(unsigned int pc,unsigned int modifiedPc){
        table[hash1(pc)][hash2(pc)].modifiedPc = modifiedPc;
    }

    unsigned int getModifyPc(unsigned int pc){
        return table[hash1(pc)][hash2(pc)].modifiedPc;
    }

};
#endif //CODE_BRANCHPREDICTOR_HPP
