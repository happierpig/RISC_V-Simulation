//
// Created by 赵先生 on 2021/6/29.
//

#ifndef RISC_V_PROGRAM_HPP
#define RISC_V_PROGRAM_HPP
#include "opReader.hpp"
#include "parser.hpp"
#include "highLevelBranchPredictor.hpp"
//#include "branchPredictor.hpp"
class program{
private:
    reader narrator;
    unsigned int reg[32];
    unsigned int pc;
    // pipeline elements
    bool endFlag;
    int bubbles;
    bool still;
    // branch predict
    branchPredictor branchJudge;
    int waiting;
private:
    // short circuit
    struct Forwarding{
        bool first,second = false;
        unsigned int rs1,rs2 = 0u;
    }forwarding;
    // register between two pipeline stage
    struct regFD{
        unsigned int pc = 0u;
        unsigned int code = 0u;
        opClass codeClass = bubble; // bubble means stop
    }rfd;
    struct regDE{
        unsigned int pc;
        opClass codeClass = bubble;
        unsigned int rs1_value;
        unsigned int rs2_value;
        unsigned int rd;unsigned int rs1;unsigned int rs2;
        unsigned int shamt;
        unsigned int imm;
    }rde;
    struct regEM{
        bool three;
        unsigned int pc;
        bool ramFlag = false;
        unsigned int ramPos = 0u;
        bool ramRead = false;
        bool ramWrite = false;
        bool sign = false;
        unsigned int data;
        unsigned int size;
//        bool pcFlag = false;
//        unsigned int newpc = 0;
        bool regFlag = false;
        unsigned int rd = 0u;
        unsigned int value = 0u;
        opClass codeClass = bubble;
        void setDefault(){
            codeClass = bubble;
            pc = ramPos = rd = data = value = size = 0u;
            ramFlag = ramRead = ramWrite = regFlag = sign = three = false;
        }
    }rem;
    struct regMW{
        bool regFlag = false;
        unsigned int rd;
        unsigned int value;
        opClass codeClass = bubble;
        void setDefault(){
            codeClass = bubble;
            regFlag = false;
            rd = value = 0u;
        }
    }rmw;
public:
    program(): narrator(1000000),pc(0),endFlag(false),bubbles(0),waiting(0),still(false){
        narrator.initialize();
    }
private:
    void makeItStill(opClass type1,opClass type2){
        if(checkNoRs(type1)) return;
        if(!readRAM(type2)) return;
        still = true;
        rde.codeClass = bubble;
    }
private:
    void IF(){
        if(endFlag){
            rfd.codeClass = end;
            return;
        }
        if(bubbles){
            rfd.codeClass = bubble;
            --bubbles;
            return;
        }
        //below branch predictor LOL
        if(modifyPc(rde.codeClass)){
            if(branchJudge.predict(rde.pc)){
                pc = rde.pc+rde.imm;
                branchJudge.setModifyPc(rde.pc,pc);
            }else{
                branchJudge.setModifyPc(rde.pc,pc);
            }
        }

        rfd.codeClass = AND;
        rfd.pc = pc;
        rfd.code = narrator.readIns(pc);
        if(rfd.code == 0x0ff00513u){
            endFlag = true;
            rfd.codeClass = end;
        }
        pc += 4;
    }

    void ID(){
        rde.codeClass = rfd.codeClass;
        if(rfd.codeClass == end || rfd.codeClass == bubble) return;
        parser ID_code(rfd.code);
        // stop for 1 clock in case of Storage
        makeItStill(ID_code.getClass(),rem.codeClass);
        if(still) return;

        unsigned int rs1 = ID_code.getrs1(),rs2 = ID_code.getrs2();
        if(rs1 >= 32) rs1 = 0;
        if(rs2 >= 32) rs2 = 0;
        rde = {rfd.pc,ID_code.getClass(),reg[rs1],reg[rs2],ID_code.getrd(),ID_code.getrs1(),ID_code.getrs2(),ID_code.getShamt(),ID_code.getimm()};

//        if(modifyPc(rde.codeClass)){bubbles = 3;rfd.codeClass = bubble;} // stop
        if(rde.codeClass == JAL) pc = rde.pc + rde.imm;
        if(rde.codeClass == JALR) pc = (rde.rs1_value+rde.imm) & (~1);
        //belows deal with Data Hazards
        if(checkNoRs(rde.codeClass)) return;
        // forwarding naming short circuit; maybe very fake(todo)
        if(rmw.codeClass != bubble && rmw.codeClass != end && rmw.regFlag && rmw.rd != 0){
            if(checkRs1(rde.codeClass)){
                if(rde.rs1 == rmw.rd){
                    forwarding.first = true;forwarding.rs1 = rmw.value;
                }
            }
            if(checkRs12(rde.codeClass)){
                if(rde.rs1 == rmw.rd){
                    forwarding.first = true;forwarding.rs1 = rmw.value;
                }else if(rde.rs2 == rmw.rd){
                    forwarding.second = true;forwarding.rs2 = rmw.value;
                }
            }
        }
        if(rem.codeClass != bubble && rem.codeClass != end && rem.regFlag && rem.rd != 0){
            if(!readRAM(rem.codeClass)){
                if(checkRs1(rde.codeClass)){
                    if(rde.rs1 == rem.rd){
                        forwarding.first = true;forwarding.rs1 = rem.value;
                    }
                }
                if(checkRs12(rde.codeClass)){
                    if(rde.rs1 == rem.rd){
                        forwarding.first = true;forwarding.rs1 = rem.value;
                    }else if(rde.rs2 == rem.rd){
                        forwarding.second = true;forwarding.rs2 = rem.value;
                    }
                }
            }
        }
    }

    void execute(){ // DE -> EM
        rem.setDefault();
        opClass codeClass = rde.codeClass;
        rem.codeClass = rde.codeClass;
        if(codeClass == end || codeClass == bubble) return;
        rem.pc = rde.pc;
        // forwarding
        if(forwarding.first) rde.rs1_value = forwarding.rs1;
        if(forwarding.second) rde.rs2_value = forwarding.rs2;
        forwarding.first = forwarding.second = false;
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
            return;
        }
        if(codeClass == JALR){
            rem.regFlag = true;
            rem.rd = rde.rd;
            rem.value = rde.pc + 4;
            return;
        }
        if(codeClass == BEQ){
            if(rde.rs1_value != rde.rs2_value) {
                if(rde.pc + 4 != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + 4;
                }
                branchJudge.modify(rde.pc,false);
            }else {
                if(rde.pc + rde.imm != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + rde.imm;
                }
                branchJudge.modify(rde.pc,true);
            }
            return;
        }
        if(codeClass == BNE){
            if(rde.rs1_value == rde.rs2_value){
                if(rde.pc + 4 != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + 4;
                }
                branchJudge.modify(rde.pc,false);
            }else {
                if(rde.pc + rde.imm != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + rde.imm;
                }
                branchJudge.modify(rde.pc,true);
            }
            return;
        }
        if(codeClass == BLT){
            if((int)rde.rs1_value < (int)rde.rs2_value){
                if(rde.pc + rde.imm != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + rde.imm;
                }
                branchJudge.modify(rde.pc,true);
            }else{
                if(rde.pc + 4 != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + 4;
                }
                branchJudge.modify(rde.pc,false);
            }
            return;
        }
        if(codeClass == BGE){
            if((int)rde.rs1_value >= (int)rde.rs2_value){
                if(rde.pc + rde.imm != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + rde.imm;
                }
                branchJudge.modify(rde.pc,true);
            }else{
                if(rde.pc + 4 != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + 4;
                }
                branchJudge.modify(rde.pc,false);
            }
            return;
        }
        if(codeClass == BLTU){
            if(rde.rs1_value < rde.rs2_value){
                if(rde.pc + rde.imm != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + rde.imm;
                }
                branchJudge.modify(rde.pc,true);
            }else{
                if(rde.pc + 4 != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + 4;
                }
                branchJudge.modify(rde.pc,false);
            }
            return;
        }
        if(codeClass == BGEU){
            if(rde.rs1_value >= rde.rs2_value){
                if(rde.pc + rde.imm != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + rde.imm;
                }
                branchJudge.modify(rde.pc,true);
            }else{
                if(rde.pc + 4 != branchJudge.getModifyPc(rde.pc)){
                    rfd.codeClass = bubble;
                    if(endFlag) endFlag = false;
                    pc = rde.pc + 4;
                }
                branchJudge.modify(rde.pc,false);
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
        if(!rem.three){
            rem.three = true;
            waiting = 3;
            still = true;
            return;
        }
        rmw.setDefault();
        rmw.codeClass = rem.codeClass;
        if(rmw.codeClass == bubble || rmw.codeClass == end) return;
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
        rem.codeClass = bubble;
    }

    void writeBack(){
        if(rmw.codeClass == bubble) return;
        if(rmw.codeClass == end) throw 1;
        rmw.codeClass = bubble;
        if(rmw.regFlag) {
            if (rmw.rd != 0) reg[rmw.rd] = rmw.value;
        }
    }

public:
    void run(){
        while(true){
            try {
                if(!still) writeBack();
                if(!still) mem();
                if(!still) execute();
                if(!still) ID();
                if(!still) IF();
                if(waiting) --waiting;
                else still = false;
            }catch (...){
                std::cout << (unsigned int)(reg[10] & (0b11111111u));
//                std::cout << "Predict Times: " << branchJudge.times() << " Predictor Efficiency : " << branchJudge.efficiency();
                break;
            }
        }
    }
};
#endif //RISC_V_PROGRAM_HPP
