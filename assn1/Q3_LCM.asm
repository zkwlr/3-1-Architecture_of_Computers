.data

str0:
  .asciiz "Enter a 32-bit unsigned integer: "
str1:
  .asciiz "The LCM of the two integers is: "
newline:
  .asciiz "\n"

.text

################################################################################
# lcm
#
# INPUTs
#   $a0: a 32-bit unsigned integer
#   $a1: a 32-bit unsigned integer
#
# OUTPUTs
#   $v0: the LCM of the two unsigned integers
#
lcm:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # FIXME
  li $v0, -1  # 초기화 값
  multu $a0, $a1 # $t1 = $a0 * $a1 
  mflo $t1       # 하위 32비트에 곱셈결과 저장
  jal gcd # $v0 = gcd($a0, $a1)
  div $t1, $v0
  mflo $v0 # $v0 = $t1/$v0 (리턴값 $v0에 하위 32비트:몫 저장)

  # $ra = stack.pop() <-- modify this part depending w.r.t. your implementation
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  jr $ra

gcd:
	#FIXME
  # if $a1 == 0; return $a0
  beq $a1, $zero, end

  # gcd($a1, $a0 % $a1) 하기전 변수 바꿔주기
  move $t0, $a0  
  move $a0, $a1  # $a0 = $a1
  div $t0, $a1   # $a0 = $t0 / $a1
  mfhi $a1       # $a1 = $a0 % $a1 ($a1에 상위 32비트:나머지 저장)

  addi $sp, $sp, -4 # 스택에 공간 할당(jal 하기전 부모함수의 $ra 저장)
  sw $ra, 0($sp)  # 스택에 $ra 저장(push)
  # else: return gcd($a1, $a0 % $a1)
  jal gcd
  lw $ra, 0($sp)   # 부모 함수 $ra 복원(pop)
  addi $sp, $sp, 4 # 스택에 공간 해제

end:
  move $v0, $a0  # return $a0
  jr $ra
################################################################################

.globl main
main:

  # stack.push($ra)
  # stack.push($s0)
  # stack.push($s1)
  addi $sp, $sp, -12
  sw $ra, 0($sp)
  sw $s0, 4($sp)
  sw $s1, 8($sp)

  # $s0 = read_int
  li $v0, 4
  la $a0, str0
  syscall
  li $v0, 5
  syscall
  move $s0, $v0

  # $s1 = read_int
  li $v0, 4
  la $a0, str0
  syscall
  li $v0, 5
  syscall
  move $s1, $v0

  # $t0 = gcd($s0, $s1)
  move $a0, $s0
  move $a1, $s1
  jal lcm
  move $t0, $v0

  # print_string(str1)
  li $v0, 4
  la $a0, str1
  syscall
  # print_int($t0)
  li $v0, 1
  move $a0, $t0
  syscall
  # print_string(newline)
  li $v0, 4
  la $a0, newline
  syscall

  # stack.pop($s1)
  # stack.pop($s0)
  # stack.pop($ra)
  lw $ra, 0($sp)
  lw $s0, 4($sp)
  lw $s1, 8($sp)
  addi $sp, $sp, 12

  # return
  jr $ra

