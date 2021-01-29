############################## count #####################
# Copyright 2019, Larry W. Clark, this document must not be copied to any other website
# Purpose:
#   Passed the address of a stirng array of characters (zero terminated) and a match 
#   character (not a string). Count returns a counts of the number of occurences
#   of the match character in the string
# Parameters:
#   i 8(%ebp)  char *pszStr - the zero terminated string to check if cMatch exist
#   i 12(%ebp) char cMatch  - the character to check if matched in *pszStr
# Locals:
#   -4(%ebp)  - iCnt - count of number of characters that match
# Notes:
#
# Return Value:
#   iCnt - The number of times char cMatch is found in *pszStr
# Register Usage
#   %ecx - contains the parm cMatch, but referenced via %cl
#   %edx - cursor pointer into the string pszStr
#
.globl count
    .type    count, @function
count:
    pushl   %ebp                # save callers %ebp
    movl    %esp, %ebp          # Set the functions %ebp
    subl    $24, %esp           # Allocate 24 bytes
    movl    $0, -4(%ebp)        # iCnt = 0
    movl    12(%ebp), %ecx      # 2nd parm -> %ecx
                                #Since second parm is a character, %cl is that character
    movl    8(%ebp), %edx       #address of 1st parm (pszStr)
.Loop:
    movb    (%edx), %al         # *pszStr -> %al
    cmpb    $0, %al             # compare *pszStr : 0
    je      .LDone              # if *pszStr is 0, jump to .LDone

    cmpb    %al, %cl            # compare cMatch : *psz
    jne     .LNotMatch          # if cMatch != *pszStr, jump to .LNotMatch
    addl    $1, -4(%ebp)        # increments iCnt by 1 if cMatch == *pszStr

.LNotMatch:
    incl    %edx                # Increment pszStr by 1
    jmp    .Loop                # Continue loop
.LDone:
    movl    -4(%ebp), %eax      # Move iCnt into %eax to be returned
    leave                       # Free the allocated memory and reset to caller's %ebp
    ret                         # return to caller
    .size    count, .-count
