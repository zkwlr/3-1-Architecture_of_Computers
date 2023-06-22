.data

info0:
  .asciiz "The string to be sorted is: "
info1:
  .asciiz "The sorted string is: "
newline:
  .asciiz "\n"

str_buf:
  .asciiz "this is a string."
str_len:
  .word 17

.text

################################################################################
# partition
#
# INPUTs
#   $a0: the starting memory address of a string (i.e., str)
#   $a1: the first index of the range of the string to be merged (i.e., left)
#   $a2: the last index of the range of the string to be merged (i.e., right)
#
partition:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # FIXME
  addi $t0, $a2, -1 # t0 = right - 1
  add $s0, $a0, $t0 # s0 = str[right - 1]
  lb $t1, ($s0) # pivot = t1 = str[right - 1]
  addi $t2, $a1, -1 # index = t2 = left - 1
  addi $t3, $a1, 0 # int i = t3 = left

loop:
  slt $t4, $t3, $a2 # if i > right - 1
  beq $t4, $zero, endloop
  add $t5, $a0, $t3 # str[i]
  lb $t6, ($t5) # str[i]
  slt $t6, $t6, $t1 # if str[i] < pivot
  bne $t6, $zero, swap
  addi $t3, $t3, 1 # i ++
  b loop # else, 다시 for문으로 돌아가기

swap:
  addi $t2, $t2, 1 # index += 1
  add $s1, $a0, $t2 # str[index]
  add $s2, $a0, $t3 # str[i]
  lb $t6, ($s1) # temp1 = str[index]
  lb $t7, ($s2) # temp2 = str[i]
  sb $t7, ($s1) # str[index] = str[i]
  sb $t6, ($s2) # str[i] = temp1
  addi $t3, $t3, 1 # i ++
  b loop # if문 조건만족으로 swap 실행 후 다시 for문으로 돌아가기

endloop:
  addi $t2, $t2, 1 # index += 1
  add $s3, $a0, $t2 # str[index]
  add $s4, $a0, $t0 # str[right - 1]
  lb $t8, ($s3) # temp1 = str[index]
  lb $t9, ($s4) # temp2 = str[right - 1]
  sb $t9, ($s3) # str[index] = str[right - 1]
  sb $t8, ($s4) # str[right - 1] = temp1
  move $v0, $t2 # return index + 1

  # $ra = stack.pop() <-- modify this part depending w.r.t. your implementation
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  jr $ra
################################################################################

################################################################################
# quickSort
#
# INPUTs
#   $a0: the starting memory address of a string (i.e., str)
#   $a1: the first index of the range of the string to be sorted (i.e., left)
#   $a2: the last index of the range of the string to be sorted (i.e., right)
#
quickSort:
  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # FIXME
  slt $t0, $a1, $a2 # if left > right
  beq $t0, $zero, end_quickSort # 재귀 탈출

  # pivot 값: 왼쪽, 오른쪽 quickSort의 기준이 된다.
  jal partition # 먼저 pivot = partition(str, left, right)로 pivot값 구하기
  
  # $a0, $a1을 재귀함수 quickSort의 인자로 값을 변경해서 넘겨주기 전에, 
  # 먼저 원래 $a0, $a1 값들(left, right 값들)을 stack에 저장해놓아야 한다.
  # 그래야 왼쪽을 정렬하는 재귀함수 quickSort가 끝나고 돌아왔을 때, 원래의 left, right 값들을
  # 다시 오른쪽을 정렬하는 두번째 재귀함수 quickSort에 $a0, $a1을 변형하여 넘겨줄
  # 수 있기 때문이다.

  # 왼쪽 quickSort 전 stack에 left, right 저장
  addi $sp, $sp, -8
  sw $a1, 0($sp) # stack.push(left)
  sw $a2, 4($sp) # stack.push(right)
  move $t1, $v0 # pivot = v0 = partition(str, left, right)
  move $a2, $t1 # right = pivot (partition 함수에서 pivot = right-1로 설정하므로, 그냥 피벗으로 설정!)

  jal quickSort # quickSort(str, left, pivot)

  # 왼쪽 quickSort 후 stack에서 left, right 가져오기
  lw $a1, 0($sp) # left = stack.pop()
  lw $a2, 4($sp) # right = stack.pop()
  addi $sp, $sp, 8

  # 오른쪽 quickSort 전 stack에 left, right 저장
  addi $sp, $sp, -8
  sw $a1, 0($sp) # stack.push(left)
  sw $a2, 4($sp) # stack.push(right)
  addi $a1, $t1, 1 # left = pivot + 1
  
  jal quickSort # quickSort(str, pivot + 1, right)

  # 오른쪽 quickSort 후 stack에서 left, right 가져오기
  lw $a1, 0($sp) # left = stack.pop()
  lw $a2, 4($sp) # right = stack.pop()
  addi $sp, $sp, 8

end_quickSort: # left > right면 함수 종료

  # stack.push($ra) <-- modify this part depending w.r.t. your implementation
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  jr $ra
################################################################################

.globl main
main:

  # stack.push($ra)
  addi $sp, $sp, -4
  sw $ra, 0($sp)

  # print_string(info0)
  li $v0, 4
  la $a0, info0
  syscall
  # print_string(str_buf)
  li $v0, 4
  la $a0, str_buf
  syscall
  # print_string(newline)
  li $v0, 4
  la $a0, newline
  syscall

  # quickSort(str_buf, 0, str_len)
  la $a0, str_buf
  li $a1, 0
  lw $a2, str_len
  jal quickSort

  # print_string(info1)
  li $v0, 4
  la $a0, info1
  syscall
  # print_string(str_buf)
  li $v0, 4
  la $a0, str_buf
  syscall
  # print_string(newline)
  li $v0, 4
  la $a0, newline
  syscall

  # stack.pop($ra)
  lw $ra, 0($sp)
  addi $sp, $sp, 4

  # return
  jr $ra

