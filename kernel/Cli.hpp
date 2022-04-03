#pragma once

struct Cli {
	char *currentPath;

	/**/ Cli();
	/**/~Cli();

	void prompt();
	void execute(char *command);
};
