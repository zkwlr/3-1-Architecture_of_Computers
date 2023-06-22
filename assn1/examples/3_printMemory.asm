################################################################################

.data

newline:
  .asciiz "\n"

array:
  .align 2
  .word 1
  .word 2
  .word 3
  .word 4
  .word 5
  .word 6
  .word 7
  .word 8

################################################################################

.text

print_array:

  # $t0 = 0; $t1 = &array[0]; $t2 = 8;
  li $t0, 0
  la $t1, array
  li $t2, 8

print_array_loop:

  # print_int(memory[$t1]);
  li $v0, 1
  lw $a0, ($t1)
  syscall

  # print_str(newline);
  li $v0, 4
  la $a0, newline
  syscall

  # $t0 += 1; $t1 += 4;
  addi $t0, $t0, 1
  addi $t1, $t1, 4

  # if ($t0 == $t2) { goto print_array_exit; }
  beq $t0, $t2, print_array_exit

  # goto print_array_loop;
  b print_array_loop

print_array_exit:

  # return;
  jr $ra

.globl main
main:

  # $s0 = $ra;
  move $s0, $ra

  # print_array();
  jal print_array

  # $ra = $s0;
  move $ra, $s0

  # return;
  jr $ra

################################################################################

