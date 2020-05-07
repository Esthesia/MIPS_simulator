#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "mips_simulator.h"

#define signExtension(x) ((int32_t)(int16_t)x)
#define RTYPE 0

int reg[32];
int program_counter = 0;

_IF_ID IF_ID;
_ID_EX ID_EX;
_EX_MEM EX_MEM;
_MEM_WB MEM_WB;
status cur_status;

int ALU_control(unsigned int function_code, int op, bool op_0, bool op_1);
int ALU_execute(unsigned int f_code, unsigned int shift_num, int ALU_con, int read_data_1, int read_data_2);

void WB();
void MEM();
void EX();
void ID();
void IF(unsigned int inst[], int pc);
void print_status();

char *register_print(int reg_num, int reg_val){
  char print_form[8];
  int c, k;
  for(c = 7; c >=0; c--){
      k = reg_val >> c;
      if(k & 1)
          print_form[7-c] = '1';
      else
          print_form[7-c] = '0';
  }
  printf("[%d] %s\n", reg_num, reg_val);
}


void code_insertion(FILE *fp, int code[]){
    char instruction[10];
    char **pos;
    int i = 0;
    while(fgets(instruction, 10, fp) != NULL){
        unsigned int value = strtol(instruction, pos, 16);
        code[i] = value;
        i++;
    }
}

void code_execution(int code[], int mode){
    if(mode == 0){
      status cur_status;
      WB();
      MEM();
      EX();
      ID();
      IF(code, program_counter);
      int reg_iter;
      for(reg_iter = 0; reg_iter < 32; reg_iter++){
        cur_status.cur_reg[reg_iter] = reg[reg_iter];
      }
      print_status();
      // return program_counter;
    }
    else if(mode == 1){
      status cur_status;
      WB();
      MEM();
      EX();
      ID();
      IF(code, program_counter);
      print_status();
      // return program_counter;
    }
}


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
  cycle_num = argv[2];
  mode = argv[3];
  fseek(instruction_file, 0, SEEK_END);
  file_size = ftell(instruction_file);
  number_of_instruction = file_size / 9;
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
    just for debugging
   */
  int i;
  for(i = 0; i < file_size; i++){
    printf("Cycle %d\n", i+1);
    printf("%08x\n\n", instruction[i]);
  }
  /*
  code execution
   */
  int j;
  for(j = 0; j < cycle_num; j++){
    code_execution(instruction, mode);
  }
  /*
    free dynamic allocating memory.
   */
  free(instruction);
  return 0;
}

void print_status(){
  printf("Cycle &d\n",cur_status.cur_cycle);
  printf("PC: %04x\n",cur_status.cur_PC);
  printf("Instruction: %08x\n\n",cur_status.cur_instruction);
  printf("Registers:\n");
  int register_iter;
  for(register_iter = 0; register_iter < 32; register_iter ++){
    register_print(register_iter, cur_status.cur_reg[register_iter]);
  }
  printf("Memory I/O: "); // not yet implemented
}

void IF(unsigned int inst[], int pc){
  unsigned int IF_UNIT = inst[pc];
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
  ID_EX.rs_value = reg[rs-1];
  ID_EX.rt_value = reg[rt-1];
  ID_EX.RT = rt;
  ID_EX.RD = rd;
  ID_EX.extension_num = signExtension(immediate_num);
  ID_EX.jump_address = (ID_INST << 6) >> 6;
  /*
    R-TYPE
   */
  if(op_code == RTYPE){
    unsigned int funct_code = (ID_INST << 26) >> 26;
    // JR instruction - need to end at ID/EX structure
    if(funct_code == 8){
      ID_EX.jump_control = TRUE;
      ID_EX.ex_control.PCSrc = TRUE;
      // when register 31(RA) does not come back - how to deal with 20200507
      ID_EX.ID_pc_num = reg[30];
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
    unsigned int ID_jump_address = (ID_INST << 6) >> 6;
    if(op_code == 3){
      reg[30] = IF_ID.IF_pc_num;
    }
    ID_EX.ID_pc_num = ID_EX.jump_address;
    ID_EX.jump_control = TRUE;
    return;
  }
  /*
    Branch instruction
    comment sidd - regDst and MemtoReg is don't care condition
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
    // ID_EX.ex_control.regDst = FALSE;
    ID_EX.ex_control.ALUSrc = TRUE;
    ID_EX.ex_control.PCSrc = FALSE;
    ID_EX.ex_control.MemRead = FALSE;
    ID_EX.ex_control.MemWrite = TRUE;
    ID_EX.ex_control.regWrite = FALSE;
    // ID_EX.ex_control.MemtoReg = FALSE;
    ID_EX.ex_control.ALUop_0 = FALSE;
    ID_EX.ex_control.ALUop_1 = FALSE;
    return;
  }
}

void EX(){
  if(jump_control){
    EX_MEM.mem_control.PCSrc = ID_EX.ex_control.PCSrc;
    EX_MEM.mem_control.MemRead = FALSE;
    EX_MEM.mem_control.MemWrite = FALSE;
    EX_MEM.mem_control.regWrite = FALSE;
    EX_MEM.mem_control.MemtoReg = FALSE;
  }
  /*
    Control value assigning
   */
  EX_MEM.mem_control.PCSrc = ID_EX.ex_control.PCSrc;
  EX_MEM.mem_control.MemRead = ID_EX.ex_control.MemRead;
  EX_MEM.mem_control.MemWrite = ID_EX.ex_control.MemWrite;
  EX_MEM.mem_control.regWrite = ID_EX.ex_control.regWrite;
  EX_MEM.mem_control.MemtoReg = ID_EX.ex_control.MemtoReg;
  /*
    Assigning Data to next step
   */
  EX_MEM.EX_pc_num = ID_EX.ID_pc_num;
  EX_MEM.EX_op_code = ID_EX.ID_op_code;
  unsigned int branch_destination = ID_EX.extension_num;
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
  int ALU_output = ALU_execute(funct, shamt, ALU_control_output, ALU_input_1, ALU_input_2);
  if(RegDst)
    decision_RegDst = ID_EX.RD;
  else
    decision_RegDst = ID_EX.RT;
  EX_MEM.ALU_result = ALU_output;
  EX_MEM.num_reg_to_write = decision_RegDst;
}

void MEM(){
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
   */
  else if(EX_MEM.mem_control.MemRead){

  }
  /*
    Memory Save instruction
   */
  else if(EX_MEM.mem_control.MemWrite){

  }
}

void WB(){
  /*
    No Register Write == No need to process WB stage
   */
  if(!MEM_WB.writeback_control.regWrite){
    return;
  }
  else{
    if(Mem_WB.writeback_control.MemtoReg){

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
 */
int ALU_control(unsigned int function_code, int op, bool op_0, bool op_1){
  int ret_val;
  if(op_0 == FALSE && op_1 == FALSE){
    ret_val = 2;
    return ret_val;
  }
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
      ret_val = 9;
      return ret_val;
    }
  }
}
/*
  ALU executor according to ALU control value
 */
int ALU_execute(unsigned int f_code, unsigned int shift_num, int ALU_con, int read_data_1, int read_data_2){
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
}
