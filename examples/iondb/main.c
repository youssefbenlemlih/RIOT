
#include <stdio.h>
#include <string.h>

#include "shell.h"

static int _hello(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("%s\n","Hello world");
    return 1;
}
static const shell_command_t shell_commands[] = {
    { "hello", "Prints hello world", _hello },
    { NULL, NULL, NULL }
};

int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
