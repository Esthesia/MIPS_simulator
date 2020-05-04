// stack related function to use at jr and jal
#include "stack.h"
void init_stack(void)
{
     stack_pointer = -1;
}

int push(int stack_value)
{
    if(stack_pointer >= 99)
    {
	printf("stack is full.\n");
	return -1;
    }
    stack_pointer ++;
    stack[stack_pointer] = stack_value;
}
int pop(void)
{
    if(stack_pointer < 0)
    {
	printf("stack is empty.\n");
    }
    int return_value = stack_pointer;
    stack_pointer --;
    return stack[return_value];
}
