//
// Created by 赵先生 on 2021/7/1.
//

#ifndef CODE_BRANCHPREDICTOR_HPP
#define CODE_BRANCHPREDICTOR_HPP
class twoBits{ // 2-bit prediction scheme
public:
    bool first,second;
    int correct,wrong;
    unsigned int modifiedPc;
public:
    twoBits():first(true),second(true),correct(1),wrong(0),modifiedPc(0u){}
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
    double efficiency()const{
        return (double) correct / (correct + wrong);
    }
};

class branchPredictor{
private:
    twoBits table[4096];
    const unsigned int lowbits = 0b111111111111u;
private:
    unsigned int hash(unsigned int pc) const {
        return pc & lowbits;
    }
public:
    branchPredictor() = default;
    bool predict(unsigned int pc) const{
        return table[hash(pc)].predict();
    }
    void modify(unsigned int pc,bool flag){
        table[hash(pc)].modify(flag);
    }
    double efficiency() const{
        unsigned int correct = 0,wrong = 0;
        for(int i = 0;i < 4096;++i){
            correct += table[i].correct;
            wrong += table[i].wrong;
        }
        return (double) correct / (correct + wrong);
    }
    void setModifyPc(unsigned int pc,unsigned int modifiedPc){
        table[hash(pc)].modifiedPc = modifiedPc;
    }
    unsigned int getModifyPc(unsigned int pc){
        return table[hash(pc)].modifiedPc;
    }
};
#endif //CODE_BRANCHPREDICTOR_HPP
