;; Fibonacci
; F(0)=0, F(1)=1
; F(n)=F(n-1)+F(n-2)
; 
; input $a0: v[], $a1: k_start, $a2: k_end
; write F(k) in v[k], k_start <= k < k_end
; assuming k>=2 && v[k] are already written for k<k_start

lui $sp, 0xffff
ori $sp, $sp, 0xfff0
ori $a0, $0, 0x100
sw $0, 0($a0)
ori $t0, $0, 1
sw $t0, 4($a0)
ori $a1, $0, 2
ori $a2, $0, 12

loop:
jal fib
addi $a1, $a1, 1
bne $a1, $a2, loop
j end

fib:
addi $sp, $sp, -4
sw $ra, 0($sp)
addi $a1, $a1, -1
jal read              #read F(n-2)
addi $t2, $v0, 0
addi $a1, $a1, -1
jal read              #read F(n-1)
add $v0, $t2, $v0
addi $a1, $a1, 2
jal write             #write F(n)
lw $ra, 0($sp)
addi $sp, $sp, 4
jr $ra

read:
sll $t0, $a1, 2
add $t1, $a0, $t0
lw $v0, 0($t1)
jr $ra

write:
sll $t0, $a1, 2
add $t1, $a0, $t0
sw $v0, 0($t1)
jr $ra

end:
beq $0, $0, end
