#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "shell.h"
#include "store.h"

void printPerson(person_t p)
{
    printf("Person{id=%.14s,lat=%f,lon=%f,status=%d,timestamp=%" PRIu64 "}\n", p.id, p.lat, p.lat, p.status, p.timestamp);
}
static int _init(int argc, char **argv)
{

    (void)argc;
    (void)argv;
    return db_init();
}
static int _close(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    return db_close();
}

static int _hello(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    printf("%s\n", "Hello world");
    return 1;
}

static int _add(int argc, char **argv)
{
    if (argc < 2)
    {
        puts("Usage: add [id]");
        return 1;
    }
    person_t p = {.lat = 3, .lon = 4, .status = 1, .timestamp = 99};
    memcpy(p.id, argv[1], 14);
    puts("Saving person:");
    printPerson(p);
    int ret = save_person(p);
    return ret;
}

static int _print(int argc, char **argv)
{
    person_t p = {.id = {'I', 'd', '\0'}, .lat = 3, .lon = 4, .status = 1, .timestamp = 99};
    puts("Saving person:");
    printPerson(p);
    int ret = save_person(p);
    return ret;
}
static const shell_command_t shell_commands[] = {
    {"init", "start the db", _init},
    {"close", "close the db", _close},
    {"add", "add person with given id", _add},
    {"print", "print all persons", _print},
    {"hello", "Prints hello world", _hello},
    {NULL, NULL, NULL}};

int main(void)
{
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
