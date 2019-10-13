#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))

void rebuild_if_modified(int argc, char *argv[])
{
    assert(argc > 0);
    const char *binary = argv[0];
    const char *source = __FILE__;

    struct stat source_stat;
    if (stat(source, &source_stat) < 0) {
        fprintf(stderr, "[ERROR] Can't check time of `%s` file: %s\n",
                source, strerror(errno));
        exit(1);
    }

    struct stat binary_stat;
    if (stat(binary, &binary_stat) < 0) {
        fprintf(stderr, "[ERROR] Can't check time of `%s` file: %s\n",
                binary, strerror(errno));
        exit(1);
    }

    if (source_stat.st_mtime > binary_stat.st_mtime) {
        printf("%s is modified. Rebuilding myself...\n", source);
        pid_t pid = fork();
        if (pid) {
            int status = 0;
            waitpid(pid, &status, 0);

            if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != 0)) {
                fprintf(stderr, "[ERROR] Could not rebuild myself!\n");
                exit(1);
            }
            // TODO: rebuild_if_modified loses original binary flags
            // TODO: rebuild_if_modified can easily go into recursion
            execlp(binary, binary, NULL);
        } else {
            printf("Building ./build\n");
            // TODO: can build.c remember which flags it was compiled with?
            execlp("gcc", "gcc", "-std=c11", "-o", binary, source, NULL);
        }
    }
}

#define LIBEBISP_NAME "libebisp.o"
#define EXECUTABLE_NAME "test"

char *const repl_cli[] = {
    "gcc",
    "-Wall", "-Werror", "-o", "repl",
    "src/repl.c",
    "src/repl_runtime.c",
    "-L.",
    "-lebisp",
    NULL
};

char *const objs_cli[] = {
    "gcc",
    "-Wall", "-Werror", "-c",
    "src/builtins.c",
    "src/expr.c",
    "src/gc.c",
    "src/interpreter.c",
    "src/parser.c",
    "src/scope.c",
    "src/std.c",
    "src/tokenizer.c",
    "src/str.c",
    NULL
};

char *const ar_cli[] = {
    "ar", "-crs",
    "libebisp.a",
    "builtins.o",
    "expr.o",
    "gc.o",
    "interpreter.o",
    "parser.o",
    "scope.o",
    "std.o",
    "tokenizer.o",
    "str.o",
    NULL
};

void runvp(const char *file, char *const cli[])
{
    printf("%s", file);
    char *const *iter = cli;
    while (*iter) {
        printf(" %s", *iter);
        iter++;
    }
    printf("\n");

    pid_t pid = fork();
    if (!pid) {
        execvp(file, cli);
    }
    int status;
    waitpid(pid, &status, 0);
    if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != 0)) {
        fprintf(stderr, "[ERROR] Build failed\n");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    rebuild_if_modified(argc, argv);
    runvp("gcc", objs_cli);
    runvp("ar", ar_cli);
    runvp("gcc", repl_cli);


    return 0;
}
