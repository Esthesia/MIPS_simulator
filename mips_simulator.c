#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "mips_simulator.h"

#define signExtension(x) ((int32_t)(int16_t)x)
#define RTYPE 0

#define NOP_IN 0x00000000
/*
  Memory Space
  1000 0000(hex)
  1000 8000(hex)
  So 8192 int array as simulating static memory space.
 */
int static_memory[8192];
int reg[32];
int program_counter = 0;

/*
  Inter Stage Structure and printing structure.
 */
_IF_ID IF_ID;
_ID_EX ID_EX;
_EX_MEM EX_MEM;
_MEM_WB MEM_WB;
status cur_status;

int ALU_control(unsigned int function_code, int op, bool op_0, bool op_1);
int ALU_execute(unsigned int f_code, unsigned int shift_num, int ALU_con, int read_data_1, int read_data_2);
void EX_pc_execute(unsigned int JR);

void IF(unsigned int inst[], int pc);
void ID();
void EX();
void MEM();
void WB();
void print_status();
void register_print(int reg_num, int reg_val);
void code_insertion(FILE *fp, int code[]);
void code_execution(int code[], int mode, int c);

/*
  HAZARD DETECTION UNIT
    DATA_FORWARDING - data hazards handling in ALU instruction
 */
void DATA_FORWADING();
/*
MAIN function
1. File reading and dynamically allocate instruction memory to start.
2. code executing given number of cycle and mode.
 */

int main(int argc, char *argv[]){
  /*
    File reading for running instruction
   */
  unsigned int file_size;
  unsigned int number_of_instruction;
  int cycle_num;
  int mode;
  FILE *instruction_file;
  instruction_file = fopen(argv[1], "r");
  cycle_num = atoi(argv[2]);
  mode = atoi(argv[3]);
  fseek(instruction_file, 0, SEEK_END);
  file_size = ftell(instruction_file);
  number_of_instruction = file_size / 10;
  fseek(instruction_file, 0, SEEK_SET);
  /*
    file insertion to execute simualtor and file close
   */
  unsigned int *instruction = (unsigned int*)malloc(sizeof(unsigned int)*number_of_instruction);
  code_insertion(instruction_file, instruction);
  fclose(instruction_file);
  /*
    Register file initialization
   */
  int register_iter;
  for(register_iter = 0; register_iter < 32; register_iter ++){
     reg[register_iter] = 0;
  }
  /*
  code execution
   */
  int j;
  for(j = 1; j <= cycle_num; j++){
    code_execution(instruction, mode, j);
  }
  /*
    free dynamic allocating memory.
   */
  free(instruction);
  return 0;
}


void IF(unsigned int inst[], int pc){
  unsigned int IF_UNIT;
  // if(ID_EX.ex_control.PCSrc == TRUE || EX_MEM.mem_control.PCSrc){
  //   IF_UNIT = NOP_IN;
  // }
  // else{
  //   IF_UNIT = inst[pc];
  // }
  IF_UNIT = inst[pc];
  pc = pc + 1;
  IF_ID.IF_pc_num = pc;
  IF_ID.instruction = IF_UNIT;
  return;
}

void ID(){
  unsigned int ID_INST = IF_ID.instruction;
  unsigned int op_code = ID_INST >> 26;
  unsigned int rs = (ID_INST << 6) >> 27;
  unsigned int rt = (ID_INST << 11) >> 27;
  unsigned int rd = (ID_INST << 16) >> 27;
  unsigned int immediate_num = (ID_INST << 16) >> 16;
  ID_EX.ID_op_code = op_code;
  ID_EX.rs_value = reg[rs];
  ID_EX.rt_value = reg[rt];
  ID_EX.RS = rs;
  ID_EX.RT = rt;
  ID_EX.RD = rd;
  ID_EX.extension_num = signExtension(immediate_num);
  ID_EX.jump_address = (ID_INST << 6) >> 6;
  /*
    R-TYPE
    When function code is 8(JAL), jump control goes to true
    Otherwise, Normal R-type control unit.
   */
  if(op_code == RTYPE){
    unsigned int funct_code = (ID_INST << 26) >> 26;
    // JR instruction - need to end at ID/EX structure
    if(funct_code == 8){
      ID_EX.jump_control = TRUE;
      ID_EX.ex_control.regDst = FALSE;
      ID_EX.ex_control.ALUSrc = FALSE;
      ID_EX.ex_control.PCSrc = TRUE;
      ID_EX.ex_control.MemRead = FALSE;
      ID_EX.ex_control.MemWrite = FALSE;
      ID_EX.ex_control.regWrite = FALSE;
      ID_EX.ex_control.MemtoReg = FALSE;
      ID_EX.ex_control.ALUop_0 = TRUE;
      ID_EX.ex_control.ALUop_1 = FALSE;
      return;
    }
    else{
      ID_EX.jump_control = FALSE;
      ID_EX.ex_control.regDst = TRUE;
      ID_EX.ex_control.ALUSrc = FALSE;
      ID_EX.ex_control.PCSrc = FALSE;
      ID_EX.ex_control.MemRead = FALSE;
      ID_EX.ex_control.MemWrite = FALSE;
      ID_EX.ex_control.regWrite = TRUE;
      ID_EX.ex_control.MemtoReg = FALSE;
      ID_EX.ex_control.ALUop_0 = FALSE;
      ID_EX.ex_control.ALUop_1 = TRUE;
      return;
    }
  }
  /*
    J-TYPE  - need to make a bubble of next IF -20200507
   */
  else if(op_code == 2 || op_code == 3){
    ID_EX.jump_control = TRUE;
    ID_EX.ex_control.regDst = FALSE;
    ID_EX.ex_control.ALUSrc = FALSE;
    ID_EX.ex_control.PCSrc = TRUE;
    ID_EX.ex_control.MemRead = FALSE;
    ID_EX.ex_control.MemWrite = FALSE;
    /*
      JAL need to write return address to R31 so RegWrite control to TRUE
     */
    ID_EX.ex_control.regWrite = FALSE;
    if(op_code == 3){
      ID_EX.ex_control.regWrite = TRUE;
    }
    ID_EX.ex_control.MemtoReg = FALSE;
    ID_EX.ex_control.ALUop_0 = TRUE;
    ID_EX.ex_control.ALUop_1 = FALSE;
    return;
  }
  /*
    Branch instruction
    comment - regDst and MemtoReg is don't care condition
   */
  else if(op_code == 4 || op_code == 5){
    ID_EX.jump_control = FALSE;
    ID_EX.ex_control.regDst = FALSE;
    ID_EX.ex_control.ALUSrc = FALSE;
    ID_EX.ex_control.PCSrc = TRUE;
    ID_EX.ex_control.MemRead = FALSE;
    ID_EX.ex_control.MemWrite = FALSE;
    ID_EX.ex_control.regWrite = FALSE;
    ID_EX.ex_control.MemtoReg = FALSE;
    ID_EX.ex_control.ALUop_0 = TRUE;
    ID_EX.ex_control.ALUop_1 = FALSE;
    return;
  }
  /*
    ALU immediate operation
   */
  else if(op_code == 8 || op_code == 10 || op_code == 11 || op_code == 12 || op_code == 13 || op_code == 15){
    ID_EX.jump_control = FALSE;
    ID_EX.ex_control.regDst = FALSE;
    ID_EX.ex_control.ALUSrc = TRUE;
    ID_EX.ex_control.PCSrc = FALSE;
    ID_EX.ex_control.MemRead = FALSE;
    ID_EX.ex_control.MemWrite = FALSE;
    ID_EX.ex_control.regWrite = TRUE;
    ID_EX.ex_control.MemtoReg = FALSE;
    ID_EX.ex_control.ALUop_0 = TRUE;
    ID_EX.ex_control.ALUop_1 = TRUE;
    return;
  }
  /*
    Load instruction
   */
  else if(op_code == 32 || op_code == 33 || op_code == 35 || op_code == 36 || op_code == 37){
    ID_EX.jump_control = FALSE;
    ID_EX.ex_control.regDst = FALSE;
    ID_EX.ex_control.ALUSrc = TRUE;
    ID_EX.ex_control.PCSrc = FALSE;
    ID_EX.ex_control.MemRead = TRUE;
    ID_EX.ex_control.MemWrite = FALSE;
    ID_EX.ex_control.regWrite = TRUE;
    ID_EX.ex_control.MemtoReg = TRUE;
    ID_EX.ex_control.ALUop_0 = FALSE;
    ID_EX.ex_control.ALUop_1 = FALSE;
    return;
  }
  /*
    save instruction
   */
  else if(op_code == 40 || op_code == 41 || op_code == 43){
    ID_EX.jump_control = FALSE;
    ID_EX.ex_control.regDst = FALSE;
    ID_EX.ex_control.ALUSrc = TRUE;
    ID_EX.ex_control.PCSrc = FALSE;
    ID_EX.ex_control.MemRead = FALSE;
    ID_EX.ex_control.MemWrite = TRUE;
    ID_EX.ex_control.regWrite = FALSE;
    ID_EX.ex_control.MemtoReg = FALSE;
    ID_EX.ex_control.ALUop_0 = FALSE;
    ID_EX.ex_control.ALUop_1 = FALSE;
    return;
  }
}

void EX(){
  /*
    Control value assigning
   */
  EX_MEM.mem_control.PCSrc = ID_EX.ex_control.PCSrc;
  EX_MEM.mem_control.MemRead = ID_EX.ex_control.MemRead;
  EX_MEM.mem_control.MemWrite = ID_EX.ex_control.MemWrite;
  EX_MEM.mem_control.regWrite = ID_EX.ex_control.regWrite;
  EX_MEM.mem_control.MemtoReg = ID_EX.ex_control.MemtoReg;
  EX_MEM.jump_control = ID_EX.jump_control;
  /*
    Assigning Data to next step
   */
  // EX_MEM.EX_pc_num = ID_EX.ID_pc_num;
  EX_MEM.EX_op_code = ID_EX.ID_op_code;
  EX_MEM.write_data = ID_EX.rt_value;
  unsigned int funct = ((unsigned int)ID_EX.extension_num << 26) >> 26;
  unsigned int shamt = ((unsigned int)ID_EX.extension_num << 21) >> 27;
  int ex_op = ID_EX.ID_op_code;
  int decision_RegDst;
  int ALU_control_output = ALU_control(funct, ex_op, ID_EX.ex_control.ALUop_0, ID_EX.ex_control.ALUop_1);
  int ALU_input_1 = ID_EX.rs_value;
  int ALU_input_2 = ID_EX.rt_value;
  /*
    ALUSrc control MUX if ALUSrc true, using signExtension.
    Otherwise use rt_value from ID_EX
   */
  if(ID_EX.ex_control.ALUSrc)
    ALU_input_2 = ID_EX.extension_num;
  int ALU_output = ALU_execute(ex_op, shamt, ALU_control_output, ALU_input_1, ALU_input_2);
  if(ID_EX.ex_control.regDst)
    decision_RegDst = ID_EX.RD;
  else
    decision_RegDst = ID_EX.RT;
  EX_MEM.ALU_result = ALU_output;
  EX_MEM.num_reg_to_write = decision_RegDst;
  EX_pc_execute(funct);
}

void MEM(){
  // IO_size unit of bits
  int IO_size;
  int memory_address = EX_MEM.ALU_result / 4;
  int access_point = EX_MEM.ALU_result % 4;
  int temp_mem_val;
  // no memory accessing occurs - Rtype
  if(!EX_MEM.mem_control.MemRead && !EX_MEM.mem_control.MemWrite){
    // All control value are zero == no need to MEM and WB stage
    if(!EX_MEM.mem_control.regWrite){
      MEM_WB.writeback_control.regWrite = FALSE;
      MEM_WB.writeback_control.MemtoReg = FALSE;
      return;
    }
    // no memory I/O
    else{
      MEM_WB.writeback_control.regWrite = TRUE;
      MEM_WB.writeback_control.MemtoReg = FALSE;
      MEM_WB.mem_data = 0;
      MEM_WB.ALU_result = EX_MEM.ALU_result;
      MEM_WB.rd_num = EX_MEM.num_reg_to_write;
      return;
    }
  }
  /*
    Memory Load instruction
    Int Array is Big Endian but this simulator access data as Little Endian (alligned)
    Access Policy - Reverse accessing.
   */
  else if(EX_MEM.mem_control.MemRead){
    MEM_WB.writeback_control.regWrite = TRUE;
    MEM_WB.writeback_control.MemtoReg = TRUE;
    MEM_WB.ALU_result = EX_MEM.ALU_result;
    MEM_WB.rd_num = EX_MEM.num_reg_to_write;
    if(EX_MEM.EX_op_code == 32){
      IO_size = 8;
      temp_mem_val = static_memory[memory_address];
      // 0 : access Most significant byte - 3: access Least significant byte
      if(access_point == 0){
        MEM_WB.mem_data = temp_mem_val >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else if(access_point == 1){
        MEM_WB.mem_data = (temp_mem_val << 8) >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else if(access_point == 2){
        MEM_WB.mem_data = (temp_mem_val << 16) >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else{
        MEM_WB.mem_data = (temp_mem_val << 24) >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
    }
    //unsigned case
    else if(EX_MEM.EX_op_code == 36){
      IO_size = 8;
      temp_mem_val = static_memory[memory_address];
      if(access_point == 0){
        MEM_WB.mem_data = ((unsigned int)temp_mem_val) >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else if(access_point == 1){
        MEM_WB.mem_data = ((unsigned int)temp_mem_val << 8) >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else if(access_point == 2){
        MEM_WB.mem_data = ((unsigned int)temp_mem_val << 16) >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else{
        MEM_WB.mem_data = ((unsigned int)temp_mem_val << 24) >> 24;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
    }
    else if(EX_MEM.EX_op_code == 33){
      IO_size = 16;
      temp_mem_val = static_memory[memory_address];
      // 0 : access upper half / 2 : access lower half
      if(access_point == 0){
        MEM_WB.mem_data = temp_mem_val >> IO_size;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else{
        MEM_WB.mem_data = (temp_mem_val << IO_size) >> IO_size;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
    }
    // unsigned case
    else if(EX_MEM.EX_op_code == 37){
      IO_size = 16;
      temp_mem_val = static_memory[memory_address];
      if(access_point == 0){
        MEM_WB.mem_data = ((unsigned int)temp_mem_val) >> IO_size;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else{
        MEM_WB.mem_data = ((unsigned int) temp_mem_val << IO_size) >> IO_size;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
    }
    else if(EX_MEM.EX_op_code == 35){
      IO_size = 32;
      MEM_WB.mem_data = static_memory[memory_address];
      MEM_WB.size_of_IO = IO_size / 8;
      return;
    }
  }
  /*
    Memory Save instruction
    using masking to change byte on position
   */
  else if(EX_MEM.mem_control.MemWrite){
    int memory_value = static_memory[memory_address];
    MEM_WB.writeback_control.regWrite = FALSE;
    MEM_WB.writeback_control.MemtoReg = FALSE;
    MEM_WB.ALU_result = EX_MEM.ALU_result;
    MEM_WB.rd_num = EX_MEM.num_reg_to_write;
    if(EX_MEM.EX_op_code == 40){
      IO_size = 8;
      temp_mem_val = EX_MEM.write_data;
      if(access_point == 0){
        temp_mem_val = temp_mem_val & 0xF000;
        static_memory[memory_address] = (memory_value & 0x0FFF) | temp_mem_val;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else if(access_point == 1){
        temp_mem_val = temp_mem_val & 0x0F00;
        static_memory[memory_address] = (memory_value & 0xF0FF) | temp_mem_val;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else if(access_point == 2){
        temp_mem_val = temp_mem_val & 0x00F0;
        static_memory[memory_address] = (memory_value & 0xFF0F) | temp_mem_val;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
      else{
        temp_mem_val = temp_mem_val & 0x000F;
        static_memory[memory_address] = (memory_value & 0xFFF0) | temp_mem_val;
        MEM_WB.size_of_IO = IO_size / 8;
        return;
      }
    }
    else if(EX_MEM.EX_op_code == 41){
      IO_size = 16;
      if(access_point == 0){
        temp_mem_val = temp_mem_val & 0xFF00;
        static_memory[memory_address] = (memory_value & 0x00FF) | temp_mem_val;
        return;
      }
      else{
        temp_mem_val = temp_mem_val & 0x00FF;
        static_memory[memory_address] = (memory_value & 0xFF00) | temp_mem_val;
        return;
      }

    }
    else if(EX_MEM.EX_op_code == 43){
      IO_size = 32;
      static_memory[memory_address] = EX_MEM.write_data;
      MEM_WB.size_of_IO = IO_size / 8;
      return;
    }
  }
}

void WB(){
  int destination_register = MEM_WB.rd_num;
  /*
    No Register Write == No need to process WB stage
   */
  if(!MEM_WB.writeback_control.regWrite){
    return;
  }
  /*
    Register Write
   */
  else{
    /*
      Memory Data to target Register
     */
    if(MEM_WB.writeback_control.MemtoReg){
      reg[destination_register] = MEM_WB.mem_data;
      return;
    }
    /*
      Arithmetic result to target Register
     */
    else{
      reg[destination_register] = MEM_WB.ALU_result;
      return;
    }
  }
}

/*
  ALU Controller return value
   0000 - AND 0
   0001 - OR 1
   0010 - ADD 2
   0110 - SUB 6
   0111 - Set on less than 7
   1000 - Set on less than unsigned 8
   1001 - shift left logical 9
   1010 - shift right logical 10
   1100 - NOR 12
   1101 - LUI 13
 */
int ALU_control(unsigned int function_code, int op, bool op_0, bool op_1){
  int ret_val;
  if(op_0 == FALSE && op_1 == FALSE){
    ret_val = 2;
    return ret_val;
  }
  /*
    For branch instruction - minus
   */
  else if(op_0 == TRUE && op_1 == FALSE){
    ret_val = 6;
    return ret_val;
  }
  else if(op_0 == FALSE && op_1 == TRUE){
    if(function_code == 32){
      ret_val = 2;
      return ret_val;
    }
    else if(function_code == 34){
      ret_val = 6;
      return ret_val;
    }
    else if(function_code == 36){
      ret_val = 0;
      return ret_val;
    }
    else if(function_code == 37){
      ret_val = 1;
      return ret_val;
    }
    else if(function_code == 39){
      ret_val = 12;
      return ret_val;
    }
    else if(function_code == 42){
      ret_val = 7;
      return ret_val;
    }
    /*
      ADD-on ALU control value for SLL/SRL/SLTU
    */
    else if(function_code == 43){
      ret_val = 8; // SLTU (to deal with unsigned value)
      return ret_val;
    }
    else if(function_code == 0){
      ret_val = 9;  // SLL
      return ret_val;
    }
    else if(function_code == 2){
      ret_val = 10; // SRL
      return ret_val;
    }
  }
  else{
    if(op == 8){
      ret_val = 2;
      return ret_val;
    }
    else if(op == 10){
      ret_val = 7;
      return ret_val;
    }
    else if(op == 11){
      ret_val = 8;
      return ret_val;
    }
    else if(op == 12){
      ret_val = 0;
      return ret_val;
    }
    else if(op == 13){
      ret_val = 1;
      return  ret_val;
    }
    else if(op == 15){
      ret_val = 13;
      return ret_val;
    }
  }
}

/*
  ALU executor according to ALU control value
 */
int ALU_execute(unsigned int op_, unsigned int shift_num, int ALU_con, int read_data_1, int read_data_2){
  int execution_output;
  EX_MEM.zero_flag = FALSE;
  if(ALU_con == 0){
    execution_output = read_data_1 & read_data_2;
    return execution_output;
  }
  else if(ALU_con == 1){
    execution_output = read_data_1 | read_data_2;
    return execution_output;
  }
  else if(ALU_con == 2){
    execution_output = read_data_1 + read_data_2;
    return execution_output;
  }
  else if(ALU_con == 6){
    // Branch case - ALU Src value is false + plain subtract arithmetic operation
    //               if value is zero, make EX_MEM.zero_flag TRUE, otherwise, default = FALSE.
    execution_output = read_data_1 - read_data_2;
    if(execution_output == 0)
      EX_MEM.zero_flag = TRUE;
      /*
        BNE case - reverting result;
       */
    else if(op_ == 5 && execution_output != 0)
      EX_MEM.zero_flag = TRUE;
    return execution_output;
  }
  else if(ALU_con == 7){
    // set on less than case - just comparing
    if(read_data_1 < read_data_2)
      return 1;
    else
      return 0;
  }
  else if(ALU_con == 8){
    // set on less than unsigned case - just comparing in unsigned state - separate for convinience
    if((unsigned int)read_data_1 < (unsigned int)read_data_2)
      return 1;
    else
      return 0;
  }
  else if(ALU_con == 9){
    execution_output = read_data_1 << shift_num;
    return execution_output;
  }
  else if(ALU_con == 10){
    execution_output = (unsigned int)read_data_1 >> shift_num;
    return execution_output;
  }
  else if(ALU_con == 12){
    execution_output = ~(read_data_1 | read_data_2);
    return execution_output;
  }
  else if(ALU_con == 13){
    execution_output = read_data_2 << 16;
    return execution_output;
  }
}

/*
  PC executor at EX stage what pc value choosing - JUMP/BRANCH case
  if PCSrc in ID_EX TRUE, No use original updated PC(PC+4)
  else just PC as usual that already in IF ID stage.
 */
void EX_pc_execute(unsigned int JR){
  if(ID_EX.ex_control.PCSrc){
    /*
      Branch TAKEN case / BNE reverting already implemented so only check zero flag
     */
    if(EX_MEM.zero_flag){
      EX_MEM.EX_pc_num = ID_EX.ID_pc_num + ID_EX.extension_num;
      return;
    }
    /*
      JR - return address at R31
     */
    else if(JR == 9){
      EX_MEM.EX_pc_num = reg[31];
      return;
    }
    /*
      JAL case
     */
    else if(ID_EX.ex_control.regWrite){
      EX_MEM.ALU_result = ID_EX.ID_pc_num;
      EX_MEM.num_reg_to_write = 31;
      EX_MEM.EX_pc_num = ID_EX.jump_address;
      return;
    }
    /*
      Plain Jump case;
     */
    else
      EX_MEM.EX_pc_num = ID_EX.jump_address;
      return;
  }
  /*
    Any other case that pc = pc + 4;
   */
  else
    EX_MEM.EX_pc_num = ID_EX.ID_pc_num;
    return;
}

/*
  Data Forwarding in ALU operations
  Condition - EX/MEM.Regwrite , MEM/WB.Regwrite is TRUE and EX/MEM, MEM/WB Rd is not $zero
 */
void DATA_FORWADING(){
    /*
    MEM HAZARD
    */
    if(MEM_WB.writeback_control.regWrite && MEM_WB.rd_num !=0){
      /*
      Case 2-A
      */
      if(ID_EX.RS == MEM_WB.rd_num){
        ID_EX.rs_value = MEM_WB.ALU_result;
        return;
      }
      /*
      Case 2-B
      */
      else if(ID_EX.RT == MEM_WB.rd_num){
        ID_EX.rt_value = MEM_WB.ALU_result;
        return;
      }
    }
    /*
      EX HAZARD
     */
    else if(EX_MEM.mem_control.regWrite && EX_MEM.num_reg_to_write !=0){
      /*
        Case 1-A
       */
      if(ID_EX.RS == EX_MEM.num_reg_to_write){
        ID_EX.rs_value = EX_MEM.ALU_result;
        return;
      }
      /*
        Case 1-B
       */
      else if(ID_EX.RT == EX_MEM.num_reg_to_write){
        ID_EX.rt_value = EX_MEM.ALU_result;
        return;
      }
    }
}

/*
  Status printing
 */
void print_status(){
  printf("Cycle %d\n",cur_status.cur_cycle);
  /*
    If the instruction is NOP(0x00000000) for stalling, print out NOP in PC and instruction
   */
  if(cur_status.cur_instruction == NOP_IN){
    printf("PC : NOP\n");
    printf("Instruction : NOP\n");
  }
  else{
    printf("PC: %04X\n",cur_status.cur_PC);
    printf("Instruction: %08X\n",cur_status.cur_instruction);
  }
  printf("Registers:\n");
  int register_iter;
  for(register_iter = 0; register_iter < 32; register_iter ++){
    register_print(register_iter, cur_status.cur_reg[register_iter]);
  }
  printf("Memory I/O: \n\n"); // not yet implemented
}

void register_print(int reg_num, int reg_val){
  printf("[%d] %08X\n", reg_num, reg_val);
}

void code_insertion(FILE *fp, int code[]){
    char instruction[10];
    int i = 0;
    while(fgets(instruction, 11, fp) != NULL){
        unsigned int value = strtol(instruction, NULL, 16);
        code[i] = value;
        i++;
    }
}

void code_execution(int code[], int mode, int c){
    if(mode == 0){
      if(EX_MEM.mem_control.PCSrc){
        program_counter = EX_MEM.EX_pc_num;
        cur_status.cur_PC = program_counter*4;
      }
      else{
        program_counter = IF_ID.IF_pc_num;
        cur_status.cur_PC = program_counter*4;
      }
      WB();
      MEM();
      EX();
      ID();
      IF(code, program_counter);
      cur_status.cur_instruction = IF_ID.instruction;
      cur_status.cur_cycle = c;
      int reg_iter;
      for(reg_iter = 0; reg_iter < 32; reg_iter++){
        cur_status.cur_reg[reg_iter] = reg[reg_iter];
      }
      print_status();
      // return program_counter;
    }
    else if(mode == 1){
      if(EX_MEM.mem_control.PCSrc){
        program_counter = EX_MEM.EX_pc_num;
        cur_status.cur_PC = program_counter*4;
      }
      else{
        program_counter = IF_ID.IF_pc_num;
        cur_status.cur_PC = program_counter*4;
      }
      WB();
      MEM();
      EX();
      ID();
      DATA_FORWADING();
      IF(code, program_counter);
      cur_status.cur_instruction = IF_ID.instruction;
      cur_status.cur_cycle = c;
      print_status();
      // return program_counter;
    }
}
