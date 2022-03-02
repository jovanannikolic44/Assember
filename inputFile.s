.global a, var_b
.extern q
.equ new_val, 0xB

.section data
.word k, a, 3
k:
    .skip 2
a:  .skip 0xA
var_b:
    .skip 3

.section text
ldr r1, $5
str r2, 0xC
ldr r3, r1
str r1, [r2]
ldr r2, [r2 + 1]
str r4, [r2 + a]
ldr r5, k
ldr r6, $k
str r5, %a
ldr sp, %k
ldr r3, %new_val
str pc, r2

call new_val
jmp var_b
jne %k
jmp 2
jeq *var_b
jeq *a
jeq *0xD
jgt *r3
jgt *[r5]
call *[r2 + 2]
call *[r3 + var_b]
call *[r6 + k]
call %new_val
call %a

push r4
pop r4

int r3

add r3, r4
and r4, r4
not r1
shl r4, r3

halt
.end