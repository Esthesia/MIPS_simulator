;; Memory operations (bytes, halfword, etc.)

addi    $s3, $0, 100
lui     $s0, 0xf0f1
ori     $s0, $s0, 0xf2f3
sw      $s0, 4($s3)
lui     $s1, 0xf4f5
ori     $s1, $s1, 0xf6f7
sw      $s1, 8($s3)
addi    $t0, $0, 3
L1:
lb      $t1, 4($t0)
lbu     $t2, 4($t0)
sll     $t5, $t0, 1
lh      $t3, 4($t5)
lhu     $t4, 4($t5)
beq     $t0, $0, L2
addi    $t0, $t0, -1
j       L1
L2:
b       L2
