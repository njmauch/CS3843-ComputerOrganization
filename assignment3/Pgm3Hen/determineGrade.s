############################## determineGrade #####################
# Purpose:
#   determineGrade determines the grade of a student based on a 
#   1000 pt scale.  
# Parameters:
#   i 8(%ebp)  int iG1       Grade 1
#   i 12(%ebp) int iG2       Grade 2
#   i 16(%ebp) int iG3       Grade 3
#   i 20(%ebp) int iG4       Grade 4
#   i 24(%ebp) int iG5       Grade 5
#   i 28(%ebp) int iG6       Grade 6
#   i 32(%ebp) int iG7       Grade 7
#   i 36(%ebp) int iG8       Grade 8
#   i 40(%ebp) int iG9       Grade 9
#   i 44(%ebp) int iG10      Grade 10
# Locals:
#   -4(%ebp)  int iqTot
#   -8(%ebp)  int iTot
# Notes:
#   Grade is determined by
#       int determineGrades (int G1,int G2,int G3,int G4,int G5,int G6,int G7,int G8,int G9,int G10)
#       {
#          int iqTot =0;
#          int iTot = 0;
#          iTot = iG1 + iG2 + iG3;
#          if(iG4 <= iG5)
#              iqTot += iG5;
#          else
#              iqTot += iG4;
#          if(iG6 <= iG7)
#              iqTot += iG7;
#          else
#              iqTot += iG6;
#          if(iG8 == 0)
#              iG8 = iG10;
#          if(iG9 == 0)
#              iG9 = iG10;
#          iTot += iqTot;
#          iTot += iG8;
#          iG9 *= 2;
#          iTot += iG9;
#          iG10 *= 3;
#          iTot += iG10;
#          return iTot;
#        }
# Return Value:
#   total grade on a 1000 pt scale
# Register Usage
#   %eax - mostly working register until after .L7 then uses registers %ebx and %edx
	.file	"determineGrade.s"
	.text
.globl determineGrade
	.type	determineGrade, @function
determineGrade:
	pushl	%ebp                #saving the caller's %ebp stack frame pointer                
	movl	%esp, %ebp          #establishing its own stack frame pointer
    pushl	%ebx                #Save the caller's %ebx since we are using %ebx
    call dumpRegs                   #call to dumpRegs
	subl	$20, %esp           #getting memory from the stack, 8 bytes locals, 12 bytes for call to hexdump, 4 bytes for %ebp, 4 bytes for return, 4 bytes for %ebx 32-12
    call dumpRegs                   #call to dumpRegs
        movl    %ebp, (%esp)        #moves ebp -> first parm for hexDump (beginning of buffer) 
        movl    $64, 4(%esp)        #moves 64 -> second parm for hexDump (buffer length)
        movl    $12, 8(%esp)        #moves 12 -> thirs parm for hexDump (bytes per line)
    call hexDump
        movl    (%esp), %ebp        #restores ebp back to ebp
	movl	$0, -4(%ebp)        #move 0 into iqTot
	movl	8(%ebp), %eax       #move iG1 -> %eax
	addl	12(%ebp), %eax      #adds iG2 to iG1 
	addl	16(%ebp), %eax      #adds iG3 to the sum of iG1 + iG2
	movl	%eax, -8(%ebp)      #moves sum of iG3 + iG2 + iG1 into iTot
	movl	20(%ebp), %eax      #moves iG4 into %eax
	cmpl	24(%ebp), %eax      #compares iG4 to iG5
	jle	.L2                 #if iG4 is less than or equal to iG5 jump to .L2
	movl	20(%ebp), %eax      #moves iG4 into %eax if iG4 is greater than iG5
	addl	%eax, -4(%ebp)      #adds iG4 to iqTot
	jmp	.L3                 #jumps to common code at .L3
.L2:                            
	movl	24(%ebp), %eax      #moves iG5 into %eax if it is greater than iG4
	addl	%eax, -4(%ebp)      #adds iG5 to iqTot
.L3:
	movl	28(%ebp), %eax      #moves iG6 into %eax
	cmpl	32(%ebp), %eax      #compares iG6 to iG7
	jle	.L4                 #if iG6 is less than or equal to iG7 jumps to .L4
	movl	28(%ebp), %eax      #moves iG6 into %eax if greater than iG7
	addl	%eax, -4(%ebp)      #adds iG6 to iqTot
	jmp	.L5                 #jumps to common code
.L4:                            
	movl	32(%ebp), %eax      #moves iG7 into %eax if greater than iG6
	addl	%eax, -4(%ebp)      #adds iG7 to iqTot
.L5:
	cmpl	$0, 36(%ebp)        #compares iG8 to 0
	jne	.L6                 #jumps to .L6 is iG8 is not equal to 0
	movl	44(%ebp), %eax      #if iG8 is equal to zero then inserts iG10 into %eax
	movl	%eax, 36(%ebp)      #moves iG10 into iG8
.L6:
	cmpl	$0, 40(%ebp)        #compares iG9 to 0
	jne	.L7                 #if iG9 is not equal to 0 then jumps to .L7
	movl	44(%ebp), %eax      #if iG9 is equal to 0 then inserts iG10 into %eax
	movl	%eax, 40(%ebp)      #moves iG10 into iG9
.L7:
	movl	-4(%ebp), %edx      #moves iqTot into %edx
	movl	-8(%ebp), %eax      #moves iTot into %eax
	addl	%edx, %eax          #adds iqTot to %eax(iTot)
	addl	36(%ebp), %eax      #adds iG8 to %eax(iTot)
    movl    40(%ebp), %ebx          #moves iG9 into %ebx
	sall	$1, %ebx            #left shift of 1 byte, multiplies iG9 by 2
	addl	%ebx, %eax          #adds iG9 to %eax(iTot)
	movl	44(%ebp), %ebx      #moves iG10 into %ebx
	leal    (%ebx,%ebx,2),%ebx  #multiplies iG10 by 3
	add     %ebx, %eax          #adds iG10 to %eax(iTot)
    movl    %eax, -8(%ebp)          #moves %eax into iTot (local variable)
	addl	$20, %esp           #deallocates memory stack
	popl	%ebx                # restore the caller's ebx
	popl	%ebp                #pops the stack to get instruction address
	ret                         #pops the stack and branches to the returned address
	.size	determineGrade, .-determineGrade
	.ident	"GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
