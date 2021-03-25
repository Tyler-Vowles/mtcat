#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "include/config.h"

struct thread_arg
{

    int pipefd[2];
    int errfile, err;
    int argno;
    size_t len;
    int (*func)(FILE *, int);
    char **files;

};

static int
rdblk(FILE *fp, int ofd)
{

    int fd;
    void *buf;
    ssize_t len;
    struct stat st;

    fd = fileno(fp);
    fstat(fd, &st);

    buf = malloc(st.st_blksize);
    if (buf == NULL)
    {
        close(fd);
        return errno;
    }

    do
    {

        len = read(fd, buf, st.st_blksize);
        write(ofd, buf, len);

    } while (len == st.st_blksize);

    /* clean up */
    free(buf);
    close(fd);
    return 0;

}

static int
rdstream(FILE* fp, int ofd)
{

    int ch;

    while ((ch = getc(fp)) != EOF)
    {
        write(ofd, &ch, 1);
    }
    
    fclose(fp);
    return 0;

}

static void *
auxiliary(void *args)
{
    
    struct thread_arg* args_form = (struct thread_arg*)args;

    for (int i = 0; i < args_form->argno; i++)
    {

        FILE *fp;

        if (args_form->files[i][0] == '-' && args_form->files[i][1] == '\0')
        {

            fp = stdin;
            stdin = fopen("/dev/null", "r");

        } else {

            fp = fopen(args_form->files[i], "r");
            if (fp == NULL)
            {

                args_form->err = errno;
                args_form->errfile = i;
                close(args_form->pipefd[1]);
                return NULL;

            }

        }

        args_form->err = args_form->func(fp, args_form->pipefd[1]);
        if (args_form->err)
        {

            args_form->errfile = i;
            close(args_form->pipefd[1]);
            fclose(fp);
            return NULL;

        }

        fclose(fp);

    }

    close(args_form->pipefd[1]);
    return NULL;

}

int
main(int argc, char **argv)
{

    pthread_t aux;
    struct thread_arg aux_args;
    aux_args.func = &rdblk;
    aux_args.err = 0;
    struct stat st;
    void *buf;
    ssize_t len;
    int err, argstart = 1, argbrk;

    /* parse arguments */
    if (argc < 2)
    {
        
        rdblk(stdin, STDOUT_FILENO);
        return EXIT_SUCCESS;
        
    }

    // check if first argument is -u flag
    if (argv[1][0] == '-' && argv[1][1] == 'u')
    {

        if (argv[1][2] == '\0')
        {

            argstart = 2;
            aux_args.func = &rdstream;
            setvbuf(stdout, NULL, _IONBF, 0);

        }

    }

    /* initalise */
    err = pipe(aux_args.pipefd);
    if (err != 0)
    {
        fprintf(stderr, "failed to create pipe %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    // break point that seperates the files to be concatenated by the threads
    argbrk = argc / 2 + argc % 2;
    aux_args.argno = argc - argbrk;
    aux_args.files = argv + argbrk;
    
    err = pthread_create(&aux, NULL, &auxiliary, &aux_args);
    if (err != 0)
    {

        close(aux_args.pipefd[0]);
        close(aux_args.pipefd[1]);
        fprintf(stderr, "failed to create thread\n");
        return EXIT_FAILURE;

    }

    /* read files */
    for (int i = argstart; i < argbrk; i++)
    {
        
        FILE* fp;

        // read from stdin
        if (argv[i][0] == '-' && argv[i][1] == '\0')
        {
            fp = stdin;
            stdin = fopen("/dev/null", "r");
        } else {

            fp = fopen(argv[i], "r");
            if (fp == NULL)
            {
                fprintf(stderr, "failed to open \"%s\": %s\n", argv[i], strerror(err));
                close(aux_args.pipefd[0]);
                return EXIT_FAILURE;
            }

        }
        

        err = aux_args.func(fp, STDOUT_FILENO);
        if (err)
        {
            fprintf(stderr, "failed to read \"%s\": %s\n", argv[i], strerror(err));
        }
        fclose(fp);

    }


    /* read pipe */
    fstat(aux_args.pipefd[0], &st);
    buf = malloc(st.st_blksize);
    if (buf == NULL)
    {
        fprintf(stderr, "failed to allocate %zu bytes: %s\n", (size_t)st.st_blksize, strerror(errno));
        return EXIT_FAILURE;
    }

    do
    {

        len = read(aux_args.pipefd[0], buf, st.st_blksize);
        write(STDOUT_FILENO, buf, len);

    } while (len == st.st_blksize);
    
    /* clean up */
    free(buf);
    close(aux_args.pipefd[0]);

    /* check if other thread ran into an error */
    if (aux_args.err)
    {

        fprintf(stderr, "failed to read \"%s\": %s", argv[argbrk + aux_args.errfile], strerror(aux_args.err));
        return EXIT_FAILURE;

    }

    pthread_join(aux, NULL);
    return EXIT_SUCCESS;
    
}