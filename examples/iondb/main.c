
#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "store.h"

static int _hello(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("%s\n", "Hello world");
    return 1;
}
/* TEST02: Read, Insert, Read, Delete, Read of Keys of test02Keys and test02Values */
static int _test(int argc, char **argv)
{
    printf("%s\n", "Hello world");
    save(0,12);
    load(-20,12);
    return 0;
}
static const shell_command_t shell_commands[] = {
    {"hello", "Prints hello world", _hello},
    {"test", "test method", _test},
    {NULL, NULL, NULL}};

int main(void)
{
    db_init();
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
