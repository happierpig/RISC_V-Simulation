#include <iostream>
#include "program.hpp"
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
int main() {
    reader test(1000000);
    test.initialize();
    int pc = 8;
    unsigned int x = test.readIns(pc);
    printBinary(x);
    return 0;
}
