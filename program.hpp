//
// Created by 赵先生 on 2021/6/29.
//

#ifndef RISC_V_PROGRAM_HPP
#define RISC_V_PROGRAM_HPP
#include "opReader.hpp"
#include "parser.hpp"
class program{
private:
    reader narrator;
    unsigned int reg[32];
    unsigned int pc;
    struct regFD{
        unsigned int pc = 0u;
        unsigned int code = 0u;
    }rfd;
    struct regDE{
        unsigned int pc;
        opClass codeClass;
        unsigned int rs1_value;
        unsigned int rs2_value;
        unsigned int rd;
        unsigned int shamt;
        unsigned int imm;
    }rde;
    struct regEM{
        unsigned int pc;
        bool ramFlag = false;
        unsigned int ramPos = 0u;
        bool ramRead = false;
        bool ramWrite = false;
        bool sign = false;
        unsigned int data;
        unsigned int size;
        bool pcFlag = false;
        unsigned int newpc = 0;
        bool regFlag = false;
        unsigned int rd = 0u;
        unsigned int value = 0u;
        void setDefault(){
            pc = ramPos = newpc = rd = data = value = size = 0u;
            ramFlag = ramRead = ramWrite = pcFlag = regFlag = sign = false;
        }
    }rem;
    struct regMW{
        bool regFlag = false;
        unsigned int rd;
        unsigned int value;
        void setDefault(){
            regFlag = false;
            rd = value = 0u;
        }
    }rmw;
public:
    program(): narrator(1000000),pc(0){
        narrator.initialize();
    }
private:


    void IF(){
        rfd.pc = pc;
        rfd.code = narrator.readIns(pc);
        pc += 4;
    }

    void ID(){
        parser ID_code(rfd.code);
        unsigned int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2();
        if(rs1 >= 32) rs1 = 0;
        if(rs2 >= 32) rs2 = 0;
        rde = {rfd.pc,ID_code.getClass(),reg[rs1],reg[rs2],ID_code.getrd(),ID_code.getShamt(),ID_code.getimm()};
    }

    void execute(){ // DE -> EM
        opClass codeClass = rde.codeClass;
        rem.setDefault();
        rem.pc = rde.pc;
        if(codeClass == LUI){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.imm;
            return;
        }
        if(codeClass == AUIPC){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.pc + rde.imm;
            return;
        }
        if(codeClass == JAL){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.pc + 4;
            rem.pcFlag = true;
            rem.newpc = rde.pc + rde.imm;
            return;
        }
        if(codeClass == JALR){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.pc + 4;
            rem.pcFlag = true;
            rem.newpc = (rde.rs1_value+rde.imm) & (~1);
            return;
        }
        if(codeClass == BEQ){
            if(rde.rs1_value != rde.rs2_value) return;
            rem.pcFlag = true;
            rem.newpc = rde.pc + rde.imm;
            return;
        }
        if(codeClass == BNE){
            if(rde.rs1_value == rde.rs2_value) return;
            rem.pcFlag = true;
            rem.newpc = rde.pc + rde.imm;
            return;
        }
        if(codeClass == BLT){
            if((int)rde.rs1_value < (int)rde.rs2_value){
                rem.pcFlag = true;
                rem.newpc = rde.pc + rde.imm;
            }
            return;
        }
        if(codeClass == BGE){
            if((int)rde.rs1_value >= (int)rde.rs2_value){
                rem.pcFlag = true;
                rem.newpc = rde.pc + rde.imm;
            }
            return;
        }
        if(codeClass == BLTU){
            if(rde.rs1_value < rde.rs2_value){
                rem.pcFlag = true;
                rem.newpc = rde.pc + rde.imm;
            }
            return;
        }
        if(codeClass == BGEU){
            if(rde.rs1_value >= rde.rs2_value){
                rem.pcFlag = true;
                rem.newpc = rde.pc + rde.imm;
            }
            return;
        }
        if(codeClass == LB){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.ramFlag = true;
            rem.ramRead = true;
            rem.sign = true;
            rem.size = 1;
            rem.ramPos = rde.rs1_value + rde.imm;
            return;
        }
        if(codeClass == LH){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.ramFlag = true;
            rem.ramRead = true;
            rem.sign = true;
            rem.size = 2;
            rem.ramPos = rde.rs1_value + rde.imm;
            return;
        }
        if(codeClass == LW){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.ramFlag = true;
            rem.ramRead = true;
            rem.sign = true;
            rem.size = 4;
            rem.ramPos = rde.rs1_value + rde.imm;
            return;
        }
        if(codeClass == LBU){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.ramFlag = true;
            rem.ramRead = true;
            rem.sign = false;
            rem.size = 1;
            rem.ramPos = rde.rs1_value + rde.imm;
            return;
        }
        if(codeClass == LHU){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.ramFlag = true;
            rem.ramRead = true;
            rem.sign = false;
            rem.size = 2;
            rem.ramPos = rde.rs1_value + rde.imm;
            return;
        }
        if(codeClass == SB){
            rem.ramFlag = true;
            rem.ramWrite = true;
            rem.size = 1;
            rem.ramPos = rde.rs1_value + rde.imm;
            rem.data = rde.rs2_value;
            return;
        }
        if(codeClass == SH){
            rem.ramFlag = true;
            rem.ramWrite = true;
            rem.size = 2;
            rem.ramPos = rde.rs1_value + rde.imm;
            rem.data = rde.rs2_value;
            return;
        }
        if(codeClass == SW){
            rem.ramFlag = true;
            rem.ramWrite = true;
            rem.size = 4;
            rem.ramPos = rde.rs1_value + rde.imm;
            rem.data = rde.rs2_value;
            return;
        }
        if(codeClass == ADDI){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value + rde.imm;
            return;
        }
        if(codeClass == SLTI){
            rem.regFlag = true;
            rem.rd = rde.rd;
            if((int)rde.rs1_value < (int)rde.imm) rem.value = 1;
            else rem.value = 0;
            return;
        }
        if(codeClass == SLTIU){
            rem.regFlag = true;
            rem.rd = rde.rd;
            if(rde.rs1_value < rde.imm) rem.value = 1;
            else rem.value = 0;
            return;
        }
        if(codeClass == XORI){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value ^ rde.imm;
            return;
        }
        if(codeClass == ORI){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value | rde.imm;
            return;
        }
        if(codeClass == ANDI){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value & rde.imm;
            return;
        }
        if(codeClass == SLLI){
            int shamt = rde.shamt;
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value << shamt;
            return;
        }
        if(codeClass == SRLI){
            int shamt = rde.shamt;
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value >> shamt;
            return;
        }
        if(codeClass == SRAI){
            int shamt = rde.shamt;
            rem.regFlag = true;
            rem.rd = rde.rd;
            unsigned int rs131 = rde.rs1_value >> 31;
            rem.value = rde.rs1_value >> shamt;
            if(rs131 == 0u) return;
            if(rs131 == 1u){
                unsigned int tmp = 0;
                for(int i = 1;i <= shamt;++i){
                    tmp <<= 1;tmp += 1;
                }
                tmp <<= (32-shamt);
                rem.value |= tmp;
            }
            return;
        }
        if(codeClass == ADD){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value+rde.rs2_value;
            return;
        }
        if(codeClass == SUB){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value-rde.rs2_value;
            return;
        }
        if(codeClass == SLL){
            rem.regFlag = true;
            rem.rd = rde.rd;
            unsigned int shamt = rde.rs2_value & (0b11111u);
            rem.value = rde.rs1_value << shamt;
            return;
        }
        if(codeClass == SLT){
            rem.regFlag = true;
            rem.rd = rde.rd;
            if((int)rde.rs1_value < (int)rde.rs2_value) rem.value = 1;
            else rem.value = 0;
            return;
        }
        if(codeClass == SLTU){
            rem.regFlag = true;
            rem.rd = rde.rd;
            if(rde.rs1_value < rde.rs2_value) rem.value = 1;
            else rem.value = 0;
            return;
        }
        if(codeClass == XOR){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value ^ rde.rs2_value;
            return;
        }
        if(codeClass == SRL){
            rem.regFlag = true;
            rem.rd = rde.rd;
            unsigned int shamt = rde.rs2_value & (0b11111u);
            rem.value = rde.rs1_value >> shamt;
            return;
        }
        if(codeClass == SRA){
            rem.regFlag = true;
            rem.rd = rde.rd;
            unsigned int shamt = rde.rs2_value & (0b11111u);
            unsigned int rs131 = rde.rs1_value >> 31;
            rem.value = rde.rs1_value >> shamt;
            if(rs131 == 0u) return;
            if(rs131 == 1u){
                unsigned int tmp = 0;
                for(int i = 1;i <= shamt;++i){
                    tmp <<= 1;tmp += 1;
                }
                tmp <<= (32-shamt);
                rem.value |= tmp;
            }
            return;
        }
        if(codeClass == OR){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value | rde.rs2_value;
            return;
        }
        if(codeClass == AND){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.rs1_value & rde.rs2_value;
            return;
        }
    }

    void mem(){ // EM -> MW
        rmw.setDefault();
        rmw.regFlag = rem.regFlag;
        rmw.rd = rem.rd;
        rmw.value = rem.value;
        if(rem.ramRead){
            rmw.regFlag = true;
            rmw.rd = rem.rd;
            rmw.value = narrator.readData(rem.ramPos,rem.size,rem.sign);
        }
        if(rem.ramWrite){
            narrator.writeData(rem.ramPos,rem.size,rem.data);
        }
        if(rem.pcFlag){
            pc = rem.newpc;
        }
    }

    void writeBack(){
        if(rmw.regFlag) {
            if (rmw.rd != 0) reg[rmw.rd] = rmw.value;
        }
    }

public:
    void run(){
        while(true) {
            IF(); // 1
            if(rfd.code == 0x0ff00513u){
                std::cout << (unsigned int)(reg[10] & (0b11111111u));
                break;
            }
            ID(); // 2
            execute();// 3
            mem(); // 4
            writeBack(); // 5
        }
    }
};
#endif //RISC_V_PROGRAM_HPP
