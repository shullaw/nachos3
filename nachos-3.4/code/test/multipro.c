#include "syscall.h"

int main() {
	Write("\nExec Adder 1", 20, ConsoleOutput);
	Exec("../test/adder");
	Yield();
	Write("\nExec Adder 2", 20, ConsoleOutput);
	Exec("../test/adder");
}
