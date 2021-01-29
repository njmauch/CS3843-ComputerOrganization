############################## substr #####################
# Copyright 2019, Larry W. Clark, this document must not be copied to any other website
# Purpose:
#   substr returns (via the fourth parameter) the substring
#   of the first parameter beginning at iBeg and ending with the
#   position before iEnd.    
# Parameters:
#   i 8(%ebp)  char *pszStr    - the zero terminated string
#   i 12(%ebp) int iBeg        - the character position beginning
#                                the substring
#   i 16(%ebp) int iEnd        - the position before this position is
#                                the end of the substring
#   o 20(%ebp) char *pszResult - the address to the result that is
#                                returned
# Locals:
#   -8(%ebp)  - iCnt - count of number of characters in result
# Notes:
#   1. Saving caller's %ebx in the stack at -4(%ebp)
# Return Value:
#   0 - parameter error (e.g., iBeg < 0)
#   >0 - length of the substring
# Register Usage
#   %edx - cursor pointer into the string pszStr
#   %ecx - cursor pointer into the string pszResult
#
.globl substr
    .type    substr, @function
substr:
    pushl   %ebp                # Saves the caller's %ebp
    movl    %esp, %ebp          # Sets the function's %ebp
    subl    $24, %esp           # Allocates 24 bytes
    movl    %ebx, -4(%ebp)      # save caller's %ebx
    movl    $0, -8(%ebp)        # iCnt = 0
    movl    20(%ebp), %ecx      # Move address of pszResult -> %ecx
    movl    12(%ebp), %esi      # Move iBeg into %ebx
    cmpl    $0, %esi            # Compares iBeg to 0
    jl      .LNull              # If iBeg is negative jump to .LNull
    cmpl    %esi, 16(%ebp)      # Compares iEnd to iBeg
    jle     .LNull              # If iBeg is less than or equal to iEnd jump to .LNull
    movl    8(%ebp), %edx       # Address of 1st parm pszStr
    movl    -8(%ebp), %eax      # Move iCnt -> %eax
    addl    %esi, %edx          # Move pszStr to beginning of where substr will start
.Loop:
    movb    (%edx), %bl         # pszStr -> %bl
    cmpl    16(%ebp), %esi      # Compares iBeg : iEnd
    je      .LDone              # If iEnd and iBeg are equal jump to .LDone
    cmpb    $0, %bl             # compare pszStr : 0
    je      .LDone              # if pszStr is 0, jump to .LDone
    movb    %bl, (%ecx)         # Moves char from pszStr into pszResult
    incl    %ecx                # Increments pszResult by 1
    incl    %edx                # Increments pszStr by 1
    incl    %esi                # Increments iBeg by 1
    incl    %eax                # Increments iCnt by 1
    jmp     .Loop
.LNull:
    movl    $0, %eax            # if iBeg < 0 or >= iEnd return 0
.LDone:
    movb    $0x00, (%ecx)       # If iBeg < 0 or >= iEnd insert 0x00 into first char
    leave                       # Free the allocated memory and reset to caller's %ebp
    ret                         # return to caller
    .size    substr, .-substr
