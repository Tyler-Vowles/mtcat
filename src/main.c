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

struct thread_arg
{

    int pipefd[2];
    int errfile, err;
    int argno;
    size_t len;
    int (*func)(int, int);
    char **argv;

};

static int
rdblk(int infd, int outfd)
{
    void *buf;
    ssize_t len;
    struct stat st;
    fstat(infd, &st);

    buf = malloc(st.st_blksize);
    if (buf == NULL)
    {
        close(infd);
        return errno;
    }

    do
    {

        len = read(infd, buf, st.st_blksize);
        write(outfd, buf, len);

    } while (len == st.st_blksize);

    /* clean up */
    free(buf);

    return 0;

}

static int
rdstream(int infd, int outfd)
{

    int ch;
    ssize_t len;

    do
    {

        len = read(infd, &ch, 1);
        write(outfd, &ch, len);

    } while (len == 1);

    return 0;

}

static void *
auxiliary(void *args)
{
    
    struct thread_arg* args_form = (struct thread_arg*)args;

    for (int i = 0; i < args_form->argno; i++)
    {

        int fd;

        if (args_form->argv[i][0] == '-' && args_form->argv[i][1] == '\0')
        {
            
            /* prevents stdin from being closed twice */
            fd = dup(STDIN_FILENO);

        } else {

            fd = open(args_form->argv[i], O_RDONLY);

        }

        /* if error let main thread deal with it */
        if (fd == -1)
        {

            args_form->err = errno;
            args_form->errfile = i;
            close(args_form->pipefd[1]);
            return NULL;

        }

        args_form->err = args_form->func(fd, args_form->pipefd[1]);
        if (args_form->err)
        {

            args_form->errfile = i;
            close(args_form->pipefd[1]);
            close(fd);
            return NULL;

        }

        close(fd);

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
        
        rdblk(STDIN_FILENO, STDOUT_FILENO);
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
    aux_args.argv = argv + argbrk;
    
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
        
        int fd;

        // read from stdin
        if (argv[i][0] == '-' && argv[i][1] == '\0')
        {
            /* prevents stdin from being closed twice */
            fd = dup(STDIN_FILENO);
        } else {

            fd = open(argv[i], O_RDONLY);

        }

        if (fd == -1)
        {
            fprintf(stderr, "failed to open \"%s\": %s\n", argv[i], strerror(err));
            close(aux_args.pipefd[0]);
            return EXIT_FAILURE;
        }
        

        err = aux_args.func(fd, STDOUT_FILENO);
        if (err)
        {
            fprintf(stderr, "failed to read \"%s\": %s\n", argv[i], strerror(err));
        }
        close(fd);

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