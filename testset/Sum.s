;; Sum
; sumFromTo(a,b) = a+(a+1)+(a+2)+...+(b-1)+b
; if a>=b, then return a

ori  $a0, $0, 1
ori  $a1, $0, 100
addi $v0, $a0, 0
slt $s0, $a0, $a1
bne $s0, $zero, L1
j L2
L1:
addi $a0, $a0, 1
add $v0, $v0, $a0
bne $a0, $a1, L1
L2:
b       L2
