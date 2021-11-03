#include "syscall.h"

int main() {
	Write("\nExec Adder 1", 20, ConsoleOutput);
	Exec("../test/adder");
	Write("\nExec Adder 2", 20, ConsoleOutput);
	Exec("../test/adder");
	Write("\nExec Adder 3", 20, ConsoleOutput);
	Join(Exec("../test/adder"));
	Write("\nExec Adder 4", 20, ConsoleOutput);
	Exec("../test/adder");
}
