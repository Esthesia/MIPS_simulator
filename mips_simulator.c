#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

#define signExtension(x) ((int32_t)(int16_t)x)
#define RTYPE 0

int reg[32];

_IF_ID IF_ID;
_ID_EX ID_EX;
_EX_MEM EX_MEM;
_MEM_WB MEM_WB;
status cur_status;


void WB(void);
void MEM(void);
void EX(void);
void ID(void);
void IF(void);
void print_status(unsigned int inst[], int pc);

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
    char *pos;
    int i = 0;
    while(fgets(instruction, 10, fp) != NULL){
        unsigned int value = strtol(instruction, pos, 16);
        code[i] = value;
        i++;
    }
}

int code_execution(int code[], int mode, int program_counter){
    if(mode == 0){
      status cur_status;
      WB();
      MEM();
      EX();
      ID();
      IF(code[], program_counter);
      int reg_iter;
      for(reg_iter = 0; reg_iter < 32; reg_iter++){
        cur_status.reg[reg_iter] = reg[reg_iter];
      }
      print_status(cur_status);
      return program_counter;
    }
    else if(mode == 1){
      status cur_status;
      WB();
      MEM();
      EX();
      ID();
      IF(,program_counter);
      print_status(cur_status);
      return program_counter;
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
  int pc_init = 0;
  for(j = 0; j < cycle_num; j++){
    pc_init = code_execution(instruction, mode, pc_init);
  }
  /*
    free dynamic allocating memory.
   */
  free(instruction);
  return 0;
}

void print_status(){
  printf("Cycle &d\n",status.cur_cycle);
  printf("PC: %04x\n",status.cur_PC);
  printf("Instruction: %08x\n\n",status.cur_instruction);
  printf("Registers:\n")
  int register_iter;
  for(register_iter = 0; register_iter < 32; register_iter ++){
    register_print(register_iter, status.cur_reg[register_iter]);
  }
  printf("Memory I/O: "); // not yet implemented
}

void IF(unsigned int inst[], int pc){
  unsigned int IF_UNIT = inst[pc];
  IF_ID.IF_pc_num = pc + 1;
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
  ID_EX.rs_value = reg[rs-1];
  ID_EX.rt_value = reg[rt-1];
  ID_EX.RT = rt;
  ID_EX.RD = rd;
  ID_EX.extension_num = signExtension(immediate_num);
  ID_EX.jump_address = (ID_INST << 6) >> 6;

  if(op_code == RTYPE)
  {
    unsigned int funct_code = (ID_INST << 26) >> 26;
    // JR instruction - need to end at ID/EX structure
    if(funct_code == 8){
      ID_EX.jump_control = TRUE;
      ID_EX.ex_control.PCSrc = TRUE;
      // when register 31(RA) does not come back - how to deal with 20200507
      ID_EX.ID_pc_num = reg[31];
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
      return;
    }
  }
  /*
    J-TYPE  - need to make a bubble of next IF -20200507
   */
  else if(op_code == 2 || op_code == 3){
    unsigned int ID_jump_address = (ID_INST << 6) >> 6;
    if(opcode == 3){
      reg[31] = IF_ID.IF_pc_num;
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
    // ID_EX.ex_control.regDst = TRUE;
    ID_EX.ex_control.ALUSrc = FALSE;
    ID_EX.ex_control.PCSrc = TRUE;
    ID_EX.ex_control.MemRead = FALSE;
    ID_EX.ex_control.MemWrite = FALSE;
    ID_EX.ex_control.regWrite = FALSE;
    // ID_EX.ex_control.MemtoReg = FALSE;
    return;
  }
  /*
    ALU immediate operation
   */
  else if(op_code == 8 || op_code == 10 || op_code == 11 || op_code == 12 || op_code == 13){
    ID_EX.jump_control = FALSE;
    ID_EX.ex_control.regDst = FALSE;
    ID_EX.ex_control.ALUSrc = TRUE;
    ID_EX.ex_control.PCSrc = FALSE;
    ID_EX.ex_control.MemRead = FALSE;
    ID_EX.ex_control.MemWrite = FALSE;
    ID_EX.ex_control.regWrite = TRUE;
    ID_EX.ex_control.MemtoReg = FALSE;
  }

}
