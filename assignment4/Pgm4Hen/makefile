# Define the list of .o files for p4
p4OBJECTS = cs3843p4Driver.o count.o countAny3.o verify.o substr.o dumpRegs.o hexDump.o hexDumpEbp.o hexDumpEsp.o
# Default Rules for building each .o
%.o: %.s 
	gcc -c $<
# Rule for building the executable
p4: ${p4OBJECTS}
	gcc -o p4 ${p4OBJECTS}
