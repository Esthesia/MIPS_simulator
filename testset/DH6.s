;; DH6

lw      $2, 20($1)
and     $4, $2, $5
or      $8, $2, $6
add     $9, $4, $2
slt     $1, $6, $7
end:
b       end
