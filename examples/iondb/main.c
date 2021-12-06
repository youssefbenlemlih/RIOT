#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "store2.h"

static int _hello(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("%s\n", "Hello world");
    return 1;
}

static int _test(int argc, char **argv)
{
    // if (argc < 2)
    // {
    //     printf("Missing arg\n Useae: save value");
    // }
    printf("%s\n", "Hello world");
    person_t p = {.id =34, .lat = 3, .lon = 4, .status = 1, .timestamp = 99};
    int ret = save_person2(p);
    return ret;
}
static const shell_command_t shell_commands[] = {
    {"hello", "Prints hello world", _hello},
    {"test", "test method", _test},
    {NULL, NULL, NULL}};

int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
