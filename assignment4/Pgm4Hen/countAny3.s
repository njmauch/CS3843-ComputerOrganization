############################## countAny3 #####################
# Copyright 2019, Larry W. Clark, this document must not be copied to any other website
# Purpose:
#   Passed the address of a string array of characters (zero terminated) and a string of 
#   three match characters.  Returns a count of the number of occurences of any of the 
#   three characters in the string.
# Parameters:
#   i 8(%ebp)  char *pszStr - zero terminated string
#   i 12(%ebp) char *psz3C  - three valid characters
# Locals:
#   -8(%ebp)  - iCnt - count of number of characters that match
# Notes:
#   1. Saving caller's %ebx in the stack at -4(%ebp)
# Return Value:
#   Number of occurences of any of the three characters in psz3C that occur in
#   pszStr
# Register Usage
#   %edx - cursor pointer into the string pszStr
#   %ecx - address of the second parameter, psz3C
#   %bl  - character from psz3C
#   %al  - character from pszStr
#
.globl countAny3
    .type    countAny3, @function
countAny3:
    pushl   %ebp                # Save the caller's %ebp
    movl    %esp, %ebp          # Set the function's %esp
    subl    $24, %esp           # Allocate 24 bytes
    movl    %ebx, -4(%ebp)      # save caller's %ebx
    movl    $0, -8(%ebp)        # iCnt = 0
    movl    8(%ebp), %edx       # Address of first parm *pszStr
#Loops through *pszStr until reaches zero byte
.LoopOne:
    movl    12(%ebp), %ecx      # *psz3C -> %ecx
    movb    (%edx), %al         # *pszStr -> %al
    cmpb    $0, %al             # Compare *pszStr : 0
    je      .LDone              # if *pszStr is 0, jump to .LDone
#Loops through *psz3C to see if match is found in *pszStr
.LoopTwo:
    movb    (%ecx), %bl         # *psz3C -> %bl
    cmpb    $0, %bl             # Compare *psz3C : 0
    je      .LDoneTwo           # If reaches end of psz3C jump to .LDoneTwo
#Check whether current pszStr char matches a char from psz3C
    cmpb    %al, %bl            # compare psz3C : pszStr
    jne     .LNotMatch          # if pszStr doesn't match any of psz3C jump to .LNotMatch
    addl    $1, -8(%ebp)        # if pszStr contains psz3C, add 1 to iCnt
#Loops to the next character in psz3C
.LNotMatch:
    incl    %ecx                # Increment *psz3C by 1
    jmp     .LoopTwo            # Jumps to next character in psz3C
.LDoneTwo:
    incl    %edx                # Increments pszStr by 1
    jmp     .LoopOne            # Loops to the next character of pszStr
.LDone:
    movl    -8(%ebp), %eax      # Moving iCnt into %eax to be returned
    movl    -4(%ebp), %ebx      # restore caller's %ebx
    leave                       # Free the allocated memory and reset to caller's %ebp
    ret                         # return to caller
    .size    countAny3, .-countAny3
 
