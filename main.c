#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define ERROR_CODE -1
#define SUCCESS_CODE 0
#define PRINT_CNT 10
#define THREAD_MSG "routine\n"
#define MAIN_MSG "main\n"
#define SEM_A_NAME "/sem_a"
#define SEM_B_NAME "/sem_b"

#define TRUE 1
#define FALSE 0

void exitWithFailure(const char *msg, int err){
    errno = err;
    fprintf(stderr, "%.256s : %.256s", msg, strerror(errno));
    exit(EXIT_FAILURE);
}

void assertSuccess(const char *msg, int errcode){
    if (errcode != SUCCESS_CODE)
        exitWithFailure(msg, errcode);
}

int initSemsSuccessAssertion(
    sem_t **sem_a,
    sem_t **sem_b,
    const char *sem_a_name,
    const char *sem_b_name,
    unsigned int value_a,
    unsigned int value_b,
    const char *err_msg){
    
    if (sem_a == NULL || sem_b == NULL ||
        sem_a_name == NULL || sem_b_name == NULL)
        return EINVAL;

    errno = SUCCESS_CODE;
    
    int is_created = FALSE;
    *sem_a = sem_open(sem_a_name, O_CREAT | O_EXCL, S_IRWXU, value_a);
    if (*sem_a == SEM_FAILED){
        if (errno == EEXIST){
            is_created = TRUE;
            *sem_a = sem_open(sem_a_name, O_CREAT, S_IRWXU, value_a);
            if (*sem_a == SEM_FAILED)
                assertSuccess("initSems", errno);
        }
        else
            assertSuccess("initSems", errno);
    }

    errno = SUCCESS_CODE;
    *sem_b = sem_open(sem_b_name, O_CREAT, S_IRWXU, value_b);
    if (*sem_b == SEM_FAILED)
        assertSuccess("initSems", errno);

    return is_created;
}

void semWaitSuccessAssertion(sem_t *sem, const char *err_msg){
    if (sem == NULL)
        return;

    int err = sem_wait(sem);
    assertSuccess(err_msg, err);
}

void semPostSuccessAssertion(sem_t *sem, const char *err_msg){
    if (sem == NULL)
        return;

    int err = sem_post(sem);
    assertSuccess(err_msg, err);
}

void semCloseSuccessAssertion(sem_t *sem, const char *err_msg){
    if (sem == NULL)
        return;

    int err = sem_close(sem);
    assertSuccess(err_msg, err);
}

void semUnlinkSuccessAssertion(const char *sem_name, const char *err_msg){
    if (sem_name == NULL)
        return;

    int err = sem_unlink(sem_name);
    assertSuccess(err_msg, err);
}
/*
    set to removing semaphores
    if kill process
*/
void sigcath(){
    sem_unlink(SEM_A_NAME);
    sem_unlink(SEM_B_NAME);
    exit(EXIT_FAILURE);
}

void iteration(
    sem_t *sem_to_post,
    sem_t *sem_to_wait,
    const char *msg,
    const char *err_msg){

    semWaitSuccessAssertion(sem_to_wait, err_msg);
    printf("%.256s", msg);
    semPostSuccessAssertion(sem_to_post, err_msg);
}

int main(int argc, char **argv){
    sem_t *sem_a, *sem_b;

    errno = SUCCESS_CODE;
    signal(SIGINT, sigcath);
    assertSuccess("main:signal", errno);

    int err = initSemsSuccessAssertion(&sem_a, &sem_b, SEM_A_NAME, SEM_B_NAME, 1, 0, "initSems");
    if (err == EINVAL)
        assertSuccess("main", err);

    const int is_created = err;
    
    sem_t *sem_post = is_created == TRUE ? sem_a : sem_b;
    sem_t *sem_wait = is_created == TRUE ? sem_b : sem_a;

    for (int i = 0; i < PRINT_CNT; ++i)
        iteration(sem_post, sem_wait, is_created == TRUE ? THREAD_MSG : MAIN_MSG, "main");
        
    if (is_created){
        semUnlinkSuccessAssertion(SEM_A_NAME, "semUnlink");
        semUnlinkSuccessAssertion(SEM_B_NAME, "semUnlink");
    }

    semCloseSuccessAssertion(sem_a, "semClose");
    semCloseSuccessAssertion(sem_b, "semClose");

    pthread_exit((void*)SUCCESS_CODE);
}
