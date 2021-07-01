//
// Created by 赵先生 on 2021/7/1.
//

#ifndef CODE_BRANCHPREDICTOR_HPP
#define CODE_BRANCHPREDICTOR_HPP
class branchPredictor{
private:
    bool first,second;
    int correct,wrong;
public:
    branchPredictor():first(true),second(true),correct(0),wrong(0){}
    void modify(bool flag){
        if(first && second){
            if(flag) return;
            else second = false;
        }
        if((!first) && (!second)){
            if(!flag) second = true;
            else second = false;
        }
        if(first && (!second)){
            if(flag) second = true;
            else first = false;
        }
        if((!first) && second){
            if(flag) second = false;
            else first = true;
        }
    }
    bool predict() const{
        return first;
    }
};
#endif //CODE_BRANCHPREDICTOR_HPP
