############################## verify #####################
# Copyright 2019, Larry W. Clark, this document must not be copied to any other website
# Purpose:
#   Passed the address of a string array and the address of an arary of valid characters.
#   Checsk to see whether each characer in the string array appears somewhere in the array
#   of valid characters.  If all of the characters in the string are valid, return NULL.
#   Otherwise returns the address of the first character that doesn't match
# Parameters:
#   i 8(%ebp)  char *pszStr    - the zero terminated string
#   i 12(%ebp) char *pszValid  - the valid characters
# Locals:
#   None
# Notes:
#   1. Saving caller's %ebx in the stack at -4(%ebp)
# Return Value:
#   NULL - all characters are in pszValid
#   address - address of first character in pszStr that isn't in pszValid
# Register Usage
#   %edx - cursor pointer into the string pszStr
#   %ecx - cursor pointer into the string pszValid
#   %eax - return address of the first character that doesn't match
#   %bl  - character in *pszValid
#
    .globl verify
    .type    verify, @function
verify:
    pushl   %ebp                # Save caller's %ebp
    movl    %esp, %ebp          # Set this function's %ebp
    subl    $24, %esp           # Allocate 24 bytes
    movl    %ebx, -4(%ebp)      # save caller's %ebx
    movl    $0, %eax            # default return value to avoid segfault
    movl    8(%ebp), %edx       # Address of first parm *pszStr
.LoopOne:
    movl    12(%ebp), %ecx      # *psz3C -> %ecx
    movb    (%edx), %al         # *pszStr -> %al
    cmpb    $0, %al             # Compare *pszStr : 0
    je      .LDone              # if *pszStr is 0, jump to .LDone
.LoopTwo:
    movb    (%ecx), %bl         # *psz3C -> %bl
    cmpb    $0, %bl             # Compare *pszValid : 0
    je      .LBreak             # If reaches end of pszValid break out of loops and go to .LBreak
    cmpb    %al, %bl            # compare pszValid : pszStr
    je      .LDoneTwo           # If pszStr is found in pszValid then jump to .LDoneTwo
    incl    %ecx                # Increment *pszValid by 1
    jmp     .LoopTwo            # Continue second loop
.LDoneTwo:
    incl    %edx                # Increment %edx by 1
    jmp     .LoopOne            # Loop to the next character in pszStr
.LBreak:
    movl    %edx, %eax          # Sets address of character not matched to %eax to be returned
.LDone:
    movl    -4(%ebp), %ebx      # restore caller's %ebx    
    leave
    ret
    .size    verify, .-verify
