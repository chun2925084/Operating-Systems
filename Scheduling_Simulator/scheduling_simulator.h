#ifndef SCHEDULING_SIMULATOR_H
#define SCHEDULING_SIMULATOR_H

#include <stdio.h>
#include <ucontext.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include "task.h"

enum TASK_STATE {
    TASK_RUNNING,
    TASK_READY,
    TASK_WAITING,
    TASK_TERMINATED
};

struct node {
    int pid;
    int Q_time;// time quantum
    long long int temp_queuing_time;
    long long int total_queuing_time;
    int waiting_time;
    char name[100];
    char state[100];
    char quantum[2];
    char priority[2];
    ucontext_t task;
    struct node *next;
    struct node *front;
};
typedef struct node Node;
Node *h_ready_q, *l_ready_q, *run_q, *waiting_q, *terminate_q;
Node *h_ready_e, *l_ready_e, *run_e, *waiting_e, *terminate_e;
Node *de_q, *temp_run_q;
int PID;
int sa_flag;
int w_flag;
ucontext_t run_mode;
struct sigaction sa;

void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);
long getCurrentTime();
void set_timer();
void add_ready_q(Node *node);
void renew_waiting_q(int time);
void renew_queuing_q(long long int time);
void add_readyq(char *name, int time, char *p);
#endif
