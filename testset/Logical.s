;; Logical (no branch, no memory I/O)
; f(a,b,c) = ~(a&b)|~(b|c)&0xffff

lui $s1, 0xabde
ori $a0, $s1, 0x9afe
ori $a1, $s1, 0xabde
ori $a2, $s1, 0xaaaa
and $t0, $a0, $a1
nor $t0, $t0, $t0
nor $t1, $a1, $a2
or $t2, $t0, $t1
andi $t2, $t2, 0xffff
end:
b       end

