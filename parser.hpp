//
// Created by 赵先生 on 2021/6/29.
//

#ifndef RISC_V_PARSER_HPP
#define RISC_V_PARSER_HPP
enum opClass{LUI,AUIPC,JAL,JALR,BEQ,BNE,BLT,BGE,BLTU,BGEU,LB,LH,LW,LBU,LHU,SB,SH,SW,ADDI,SLTI,SLTIU,XORI,ORI,ANDI,SLLI,SRLI,SRAI,ADD,SUB,SLL,SLT,SLTU,XOR,SRL,SRA,OR,AND,end,bubble};
class parser{
private:
    unsigned int code;
    opClass codeClass;
public:
    parser() = delete;
    explicit parser(unsigned int tmp):code(tmp),codeClass(bubble){
        unsigned int opCode = tmp & (0b1111111u);
        unsigned int func = (tmp >> 12) & (0b111u);
        unsigned int func7 = (tmp >> 25) & (0b1111111u);
        if(opCode == 0b0110111u) codeClass = LUI;
        if(opCode == 0b0010111u) codeClass = AUIPC;
        if(opCode == 0b1101111u) codeClass = JAL;
        if(opCode == 0b1100111u) codeClass = JALR;
        if(opCode == 0b1100011u){
            if(func == 0b000u) codeClass = BEQ;
            if(func == 0b001u) codeClass = BNE;
            if(func == 0b100u) codeClass = BLT;
            if(func == 0b101u) codeClass = BGE;
            if(func == 0b110u) codeClass = BLTU;
            if(func == 0b111u) codeClass = BGEU;
        }
        if(opCode == 0b0000011u){
            if(func == 0b000u) codeClass = LB;
            if(func == 0b001u) codeClass = LH;
            if(func == 0b010u) codeClass = LW;
            if(func == 0b100u) codeClass = LBU;
            if(func == 0b101u) codeClass = LHU;
        }
        if(opCode == 0b0100011u){
            if(func == 0b000u) codeClass = SB;
            if(func == 0b001u) codeClass = SH;
            if(func == 0b010u) codeClass = SW;
        }
        if(opCode == 0b0010011u){
            if(func == 0b000u) codeClass = ADDI;
            if(func == 0b010u) codeClass = SLTI;
            if(func == 0b011u) codeClass = SLTIU;
            if(func == 0b100u) codeClass = XORI;
            if(func == 0b110u) codeClass = ORI;
            if(func == 0b111u) codeClass = ANDI;
            if(func == 0b001u) codeClass = SLLI;
            if(func == 0b101u){
                if(func7 == 0u) codeClass = SRLI;
                if(func7 == 0b0100000u) codeClass = SRAI;
            }
        }
        if(opCode == 0b0110011){
            if(func == 0u){
                if(func7 == 0u) codeClass = ADD;
                if(func7 == 0b0100000u) codeClass = SUB;
            }
            if(func == 1u) codeClass = SLL;
            if(func == 0b010u) codeClass = SLT;
            if(func == 0b011u) codeClass = SLTU;
            if(func == 0b100u) codeClass = XOR;
            if(func == 0b101u){
                if(func7 == 0u) codeClass = SRL;
                if(func7 == 0b0100000u) codeClass = SRA;
            }
            if(func == 0b110u) codeClass = OR;
            if(func == 0b111u) codeClass = AND;
        }
//        if(codeClass == nullop) throw 1;
    }
public:
    opClass getClass(){return codeClass;}
    unsigned int getrd(){
        return (code >> 7) & (0b11111u);
    }
    unsigned int getrs1(){
        return (code >> 15) & (0b11111u);
    }
    unsigned int getrs2(){
        return (code >> 20) & (0b11111u);
    }
    unsigned int getShamt(){
        return (code >> 20) &(0b11111u);
    }
    unsigned int getimm(){
        unsigned int imm = 0;
        unsigned int inst31 = code >> 31;
        if(codeClass == LUI || codeClass == AUIPC){ // type U
            imm = code;
            imm >>= 12;
            imm <<= 12;
            return imm;
        }
        if(codeClass == JAL){ // type J
            if(inst31 == 0u){
                imm += (code >> 12) & (0b11111111u);
                imm <<= 1;
                imm += (code >> 20) & (1u);
                imm <<= 10;
                imm += (code >> 21) & (0b1111111111u);
                imm <<= 1;
                return imm;
            }
            if(inst31 == 1u){
                imm = 0b111111111111u;
                imm <<= 8;
                imm += (code >> 12) & (0b11111111u);
                imm <<= 1;
                imm += (code >> 20) & (1u);
                imm <<= 10;
                imm += (code >> 21) & (0b1111111111u);
                imm <<= 1;
                return imm;
            }
        }
        if(codeClass == JALR || codeClass == LB || codeClass == LH || codeClass == LW || codeClass == LBU || codeClass == LHU || codeClass == ADDI || codeClass == SLTI || codeClass == SLTIU || codeClass == XORI || codeClass == ORI || codeClass == ANDI || codeClass == SLLI || codeClass == SRLI || codeClass == SRAI){ //type I
            if(inst31 == 0u){
                imm += (code >> 20) & (0b11111111111);
                return imm;
            }
            if(inst31 == 1u){
                imm = 0b111111111111111111111u;
                imm <<= 11;
                imm += (code >> 20) & (0b11111111111);
                return imm;
            }
        }
        if(codeClass == BEQ || codeClass == BNE || codeClass == BLT || codeClass == BGE || codeClass == BLTU || codeClass == BGEU){ // type B
            if(inst31 == 0u){
                imm += (code >> 7) & (1u);
                imm <<= 6;
                imm += (code >> 25) & (0b111111u);
                imm <<= 4;
                imm += (code >> 8) & (0b1111u);
                imm <<= 1;
                return imm;
            }
            if(inst31 == 1u){
                imm = 0b11111111111111111111u;
                imm <<= 1;
                imm += (code >> 7) & (1u);
                imm <<= 6;
                imm += (code >> 25) & (0b111111u);
                imm <<= 4;
                imm += (code >> 8) & (0b1111u);
                imm <<= 1;
                return imm;
            }
        }
        if(codeClass == SB || codeClass == SH || codeClass == SW){ // type S
            if(inst31 == 0u){
                imm += (code >> 25) & (0b111111u);
                imm <<= 4;
                imm += (code >> 8) & (0b1111u);
                imm <<= 1;
                imm += (code >> 7) & (1u);
                return imm;
            }
            if(inst31 == 1u){
                imm = 0b111111111111111111111u;
                imm <<= 6;
                imm += (code >> 25) & (0b111111u);
                imm <<= 4;
                imm += (code >> 8) & (0b1111u);
                imm <<= 1;
                imm += (code >> 7) & (1u);
                return imm;
            }
        }
        if(codeClass == ADD || codeClass == SUB || codeClass == SLL || codeClass == SLT || codeClass == SLTU || codeClass == XOR || codeClass == SRL || codeClass == SRA || codeClass == OR || codeClass == AND){ // type R
            return 0;
        }
    }

};

bool modifyPc(opClass type){
    return (type == JAL || type == JALR || type == BEQ || type == BNE || type == BLT || type == BGE || type == BLTU || type == BGEU);
}
bool readRAM(opClass type){
    return (type == LB || type == LH || type == LW || type == LBU || type == LHU );
}
bool checkRs1(opClass type){
    return (type == JALR  || type == LB || type == LH || type == LW || type == LBU || type == LHU || type == ADDI || type == SLTI || type == SLTIU || type == XORI || type == ORI || type == ANDI || type == SLLI || type == SRLI || type == SRAI);
}
bool checkRs12(opClass type){
    return (type==BEQ||type==BNE||type==BLT||type==BGE||type==BLTU||type==BGEU||type==SB||type==SH||type==SW||type==ADD||type==SUB||type==SLL||type==SLT||type==SLTU||type==XOR||type==SRL||type==SRA||type==OR||type==AND);
}
bool checkNoRs(opClass type){
    return (type==LUI||type==AUIPC||type==JAL);
}
#endif //RISC_V_PARSER_HPP
