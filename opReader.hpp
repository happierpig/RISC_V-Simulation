//
// Created by 赵先生 on 2021/6/29.
//

#ifndef RISC_V_OPREADER_HPP
#define RISC_V_OPREADER_HPP
#include <cstdio>

class reader{
private:
    unsigned int ptr;
    unsigned char* opCode;
public:
    reader() = delete;
    explicit reader(int Size):ptr(0){
        opCode = new unsigned char[Size];
    }
    ~reader(){delete [] opCode;}
private:
    bool check(char ch){
        if(ch >= '0' && ch <= '9') return true;
        else if(ch >= 'A' && ch <= 'F') return true;
        else return false;
    }
    int readByte(){ //Hex->Binary
        int ans = 0;
        char ch = getchar();
        if(ch == '@') throw 1;
        if(ch == '#') throw '1';
        while(!check(ch)){
            ch = getchar();
            if(ch == '@') throw 1;
            if(ch == '#') throw '1';
        }
        while(check(ch)){
            ans <<= 4;
            if(ch >= '0' && ch <= '9') ans += (int)(ch - '0');
            else ans += (int)(ch - 'A')+10;
            ch = getchar();
        }
        return ans;
    }
    void printBinary(int x){
        int s[100] = {0};int top = 0;
        while(x > 0){
            if(x & 1) s[++top] = 1;
            else s[++top] = 0;
            x >>= 1;
        }
        while(top) std::cout << s[top--];
        std::cout << std::endl;
    }
    unsigned int signExtend(unsigned int data,int size){
        int bits = size << 3;
        if(bits == 32) return data;
        unsigned int headBit = data >> (bits-1);
        if(headBit == 0u) return data;
        if(headBit == 1u){
            unsigned int tmp = 0;
            for(int i = 1;i <= (32-bits);++i){
                tmp<<=1;tmp+=1;
            }
            tmp <<= bits;
            return data | tmp;
        }
    }
public:
    void initialize(){
        char ch = getchar();
        while(true) {
            ptr = readByte();
            while (true) {
                try {
                    int x = readByte();
                    opCode[ptr++] = x;
                } catch (int) {
                    break;
                }catch (char){
                    return;
                }
            }
        }
    }
    unsigned int readIns(int pc){
        unsigned int ans = 0;
        for(int i = 3;i >= 0;--i){
            ans <<= 8;
            ans += (int)opCode[pc+i];
        }
        return ans;
    }
    unsigned int readData(int pos,int size,bool sign){
        //bits: 1(size) == 1(char) == 1 bytes = 8 bits
        unsigned int ans = 0;
        for(int i = size-1;i >= 0;--i){
            ans <<= 8;
            ans += (int)opCode[pos+i];
        }
        if(!sign) return ans;
        else return signExtend(ans,size);
    }
    void writeData(int pos,int size,unsigned int data){
        for(int i = 0;i < size;++i){
            unsigned int tmp = data & (0b11111111u);
            data >>= 8;
            opCode[pos+i] = (unsigned char) tmp;
        }
    }
};
#endif //RISC_V_OPREADER_HPP
