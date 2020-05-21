TARGET = simulator
SOURCES = mips_simulator.c mips_simulator.h

$(TARGET): $(SOURCE)
	gcc -o $(TARGET) -O2 -Wall $(SOURCES)


Fibtest:	$(TARGET)
	./$(TARGET) binary/Fibonacci.txt 500 1 > out-Fibonacci.txt

Endtest:	$(TARGET)
	./$(TARGET) binary/End.txt 100 1 > out-End.txt

Logicaltest:	$(TARGET)
	./$(TARGET) binary/Logical.txt 100 1 > out-Logical.txt

Memorytest:	$(TARGET)
	./$(TARGET) binary/Memory.txt 100 1 > out-Memory.txt

Swaptest:	$(TARGET)
	./$(TARGET) binary/Swap.txt 200 1 > out-Swap.txt

SUtest:	$(TARGET)
	./$(TARGET) binary/SU.txt 100 1 > out-SU.txt

Sumtest:	$(TARGET)
	./$(TARGET) binary/Sum.txt 1000 1 > out-Sum.txt

clean:
	rm out-*.txt
	rm $(TARGET)

test: Fibtest Endtest Logicaltest Memorytest Swaptest SUtest Sumtest
