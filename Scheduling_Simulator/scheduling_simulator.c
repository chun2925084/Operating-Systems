#include "scheduling_simulator.h"
ucontext_t shell_mode;
ucontext_t terminate;
ucontext_t current;
ucontext_t run;
struct itimerval timer, old;
int Time;
ucontext_t main_ctx, test_ctx;

void hw_suspend(int msec_10)
{
    set_timer(0);
    strcpy(run_q->state,"TASK_WAITING");
    run_q->waiting_time = msec_10*10;
    if(waiting_e == NULL) {
        waiting_q = waiting_e = run_q;
    } else {
        waiting_e->next = run_q;
        run_q->front = waiting_e;
        waiting_e = run_q;
    }
    Node *temp = run_q;
    run_q = NULL;
    swapcontext(&(temp->task), &run);
    return;
}

void hw_wakeup_pid(int pid)
{
    if(pid<0) {
        return;
    }
    Node *task = waiting_q;
    while(task) {
        if(task->pid == pid) {
            strcpy(task->state,"TASK_READY");
            task->waiting_time = 0;
            if(task->next!=NULL) {
                /*Not the end of waiting_q*/
                if(task->front!=NULL) {
                    /*Not the front of waiting_q*/
                    task->front->next = task->next;
                    task->next->front = task->front;
                    task->front = NULL;
                    task->next = NULL;
                } else { //first one
                    task->next->front = NULL;
                    waiting_q = task->next;
                    task->next = NULL;
                }
            } else {
                if(task->front!=NULL) { //the last one
                    task->front->next = NULL;
                    waiting_e = task->front;
                    task->front = NULL;
                } else {
                    waiting_q = NULL;
                    waiting_e = NULL;
                }
            }
            /*if there is the only one node in the wainting_q do nothing*/
            task->next = NULL;
            Node *n = task;
            add_ready_q(n);
            task = NULL;
            break;
        }

    }
    return;
}

int hw_wakeup_taskname(char *task_name)
{
    Node *task = waiting_q;
    Node *temp_q = task;
    int count = 0;
    while(task) {
        temp_q = task;
        task = task->next;
        if(!strcmp(temp_q->name, task_name)) {
            count++;
            strcpy(temp_q->state,"TASK_READY");
            temp_q->waiting_time = 0;
            if(temp_q->next!=NULL) {
                /*Not the end of waiting_q*/
                if(temp_q->front!=NULL) {
                    /*Not the front of waiting_q*/
                    temp_q->front->next = temp_q->next;
                    temp_q->next->front = temp_q->front;
                    temp_q->front = NULL;
                    temp_q->next = NULL;
                } else { //first one
                    temp_q->next->front = NULL;
                    waiting_q = temp_q->next;
                    temp_q->next = NULL;
                }
            } else {
                if(temp_q->front!=NULL) { //the last one
                    temp_q->front->next = NULL;
                    waiting_e = temp_q->front;
                    temp_q->front = NULL;
                } else {
                    waiting_q = NULL;
                    waiting_e = NULL;
                }
            }
            /*if there is the only one node in the wainting_q do nothing*/
            temp_q->next = NULL;
            Node *n = temp_q;
            add_ready_q(n);
            temp_q = NULL;
        }

    }

    return count;
}

int hw_task_create(char *task_name)
{

    //printf("hw_task_create\n");
    if(strcmp(task_name, "task1") || strcmp(task_name, "task2") || strcmp(task_name, "task3") || strcmp(task_name, "task4") || strcmp(task_name, "task5")) {
        add_readyq(task_name,10, "L");
        return PID; // the pid of created task name
    } else {
        return -1;
    }
}

long getCurrentTime()
{

    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;

}

void timer_handler()
{

    //static int count = 0;
    strcpy(run_q->state,"TASK_READY");
    renew_waiting_q(run_q->Q_time);
    renew_queuing_q(run_q->Q_time);
    add_ready_q(run_q);
    Node *t = run_q;
    //printf("time = %d\n", timer.it_value.tv_usec);
    run_q = NULL;
    current = run;
    swapcontext(&(t->task), &run);

}

void renew_waiting_q(int time)
{
    Node *temp = waiting_q;
    while(temp) {
        //printf("temp = %s\n",temp->name);
        //Node *remove = temp;
        de_q = temp;
        temp->waiting_time = temp->waiting_time - time;
        temp = temp->next;
        if(de_q->waiting_time<=0) {
            //printf("waiting_q_name = %s, waiting_time =  %d\n", temp->name, temp->waiting_time);
            if(de_q->next!=NULL) {
                /*Not the end of waiting_q*/
                if(de_q->front!=NULL) {
                    /*Not the front of waiting_q*/
                    de_q->front->next = de_q->next;
                    de_q->next->front = de_q->front;
                    de_q->front = NULL;
                    de_q->next = NULL;
                } else { //first one
                    de_q->next->front = NULL;
                    waiting_q = de_q->next;
                    de_q->next = NULL;
                }
            } else {
                if(de_q->front!=NULL) { //the last one
                    de_q->front->next = NULL;
                    waiting_e = de_q->front;
                    de_q->front = NULL;
                } else {
                    waiting_q = NULL;
                    waiting_e = NULL;
                }
            }
            /*if there is the only one node in the wainting_q do nothing*/
            de_q->next = NULL;
            strcpy(de_q->state, "TASK_READY");
            Node *n = de_q;
            add_ready_q(n);
            de_q = NULL;
            w_flag = 0;

        }
    }
}

void renew_queuing_q(long long int t)
{

    Node* temp = h_ready_q;
    while(temp) {
        temp->total_queuing_time += (getCurrentTime()-temp->temp_queuing_time);
        temp->temp_queuing_time = getCurrentTime();
        temp = temp->next;
    }
    temp = l_ready_q;
    while(temp) {
        temp->total_queuing_time += (getCurrentTime()-temp->temp_queuing_time);
        temp->temp_queuing_time = getCurrentTime();
        temp = temp->next;
    }
    return;

}

void signal_handler(int signum)
{
    switch(signum) {
    case SIGALRM:
        //printf("time = %d\n", timer.it_value.tv_usec);
        timer_handler();
        break;
    case SIGTSTP:
        printf("Ctrl+Z\n");
        sa_flag = 1;
        Time = timer.it_value.tv_usec;
        set_timer(0);
        //printf("Time = %d\n", Time);
        getcontext(&current);
        swapcontext(&current, &shell_mode);
        Node* temp = h_ready_q;
        while(temp) {
            temp->temp_queuing_time = getCurrentTime();
            temp = temp->next;
        }
        temp = l_ready_q;
        while(temp) {
            temp->temp_queuing_time = getCurrentTime();
            temp = temp->next;
        }
        break;
    }


}

void set_timer(int time)
{

    //struct itimerval timer;
    /*configure the timer to expire after 1 sec*/
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = time*1000;
    /*......and every 1 sec after that*/
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    /*start a virtual timer. it count down whenever this process is executing. */
    setitimer(ITIMER_REAL, &timer, &old);
}

Node* remove_ready_q()
{

    Node *temp;
    if(h_ready_e == NULL) {
        //printf("Readyq is empty\n");
        temp = NULL;
    } else {
        if(h_ready_q != h_ready_e) {
            temp = h_ready_q;
            h_ready_q = h_ready_q->next;
        } else if(h_ready_q == h_ready_e) {
            temp = h_ready_q;
            //printf("Readyq is the only one \n");
            h_ready_q = h_ready_e = NULL;
        }
        temp->next = NULL;
    }
    if(temp == NULL) {
        if(l_ready_e == NULL) {
            //printf("Readyq is empty\n");
            temp = NULL;
        } else {
            if(l_ready_q != l_ready_e) {
                temp = l_ready_q;
                l_ready_q = l_ready_q->next;
            } else if(l_ready_q == l_ready_e) {
                temp = l_ready_q;
                //printf("Readyq is the only one \n");
                l_ready_q = l_ready_e = NULL;
            }
            temp->next = NULL;
        }
    }
    if(temp) {
        temp->total_queuing_time += (getCurrentTime()-temp->temp_queuing_time);
        //	printf("remove = %lld, remove current = %ld\n", temp->total_queuing_time, getCurrentTime());
    }
    return temp;
}

void simulating()
{

    //printf("Simulating....\n");
    Node* n = h_ready_q;
    while(n) {
        n->temp_queuing_time = getCurrentTime();
        n = n->next;
    }
    n = l_ready_q;
    while(n) {
        n->temp_queuing_time = getCurrentTime();
        n = n->next;
    }
    while(1) {
        run_q = remove_ready_q();
        while(run_q == NULL) {
            if(waiting_q == NULL) {
                //printf("there is no task\n");
                setcontext(&shell_mode);
                break;
            } else {
                renew_waiting_q(10);
                //printf("waiting for watingq\n");
                run_q = remove_ready_q();
            }
            //return;
        }
        strcpy(run_q->state, "TASK_RUNNING");
        set_timer(run_q->Q_time);
        current = run_q->task;

        getcontext(&run);
        swapcontext(&run, &(run_q->task));
        run_q = NULL;
    }
}

void add_terminate_q()
{
    //Node *node = run_q;
    //printf("run_q = %s\n", run_q->name);
    //sleep(1);
    if(terminate_e == NULL) {
        terminate_q = terminate_e = run_q;//node;
    } else {
        terminate_e->next = run_q;
        terminate_e = run_q;
        terminate_e->next = NULL;
    }
    Node *temp = terminate_q;
    while(temp) {
        //printf("terminated task %s, %d\n", temp->name, temp->pid);
        temp = temp->next;
    }
    //run_q = NULL;
}

void termination()
{

    //printf("///////////////////////\n");
    while(1) { //???????????
        set_timer(0);
        if(run_q == NULL) return;
        strcpy(run_q->state,"TASK_TERMINATED");
        add_terminate_q();
        swapcontext(&terminate, &run);
    }
}

void add_readyq(char* name, int time, char *p)
{

    Node *n;
    n = (Node*)malloc(sizeof(Node));
    strcpy(n->name, name);
    n->Q_time = time;
    strcpy(n->quantum, "S");
    if(time == 20) {
        strcpy(n->quantum, "L");
        //	printf("n = %ld\n",strlen(n->quantum));
    } else if(time == 10) {
        strcpy(n->quantum, "S\0");
    }
    n->next = NULL;
    n->pid = ++PID;
    n->total_queuing_time = 0;
    n->temp_queuing_time = getCurrentTime();
    strcpy(n->state, "TASK_READY");
    getcontext(&(n->task));
    char *stack = malloc(8192);
    n->task.uc_stack.ss_sp = stack;//malloc(8192);
    n->task.uc_stack.ss_size = 8192;
    n->task.uc_stack.ss_flags = 0;
    n->task.uc_link = &terminate;
    if(!strcmp(name,"task1")) {
        makecontext(&(n->task), task1, 0);
    } else if(!strcmp(name,"task2")) {
        makecontext(&(n->task), task2, 0);
    } else if(!strcmp(name,"task3")) {
        makecontext(&(n->task), task3, 0);
    } else if(!strcmp(name,"task4")) {
        makecontext(&(n->task), task4, 0);
    } else if(!strcmp(name,"task5")) {
        makecontext(&(n->task), task5, 0);
    } else if(!strcmp(name,"task6")) {
        makecontext(&(n->task), task6, 0);
    }
    //printf("p - %s\n", p);
    if(!strcmp(p, "H")) {
        strcpy(n->priority, "H");
        if(h_ready_e == NULL) {
            h_ready_q = h_ready_e = n;
            //	printf("the only one readyq\n");
        } else {
            h_ready_e->next = n;
            h_ready_e = n;
        }
    } else if(!strcmp(p, "L")) {
        strcpy(n->priority, "L");
        if(l_ready_e == NULL) {
            l_ready_q = l_ready_e = n;
        } else {
            l_ready_e->next = n;
            l_ready_e = n;
        }
    }
}

void add_ready_q(Node *n)
{
    Node *node = n;
    node->temp_queuing_time = getCurrentTime();
    if(!strcmp(node->priority, "H")) {
        if(h_ready_e == NULL) {
            h_ready_q = h_ready_e = node;
        } else {
            h_ready_e->next = node;
            h_ready_e = node;
            h_ready_e->next = NULL;
        }
    } else if(!strcmp(node->priority, "L")) {
        if(l_ready_e == NULL) {
            l_ready_q = l_ready_e = node;
        } else {
            l_ready_e->next = node;
            l_ready_e = node;
            l_ready_e->next = NULL;
        }
    }

}




void shell()
{

    char temp[100];
    char task_name[100];
    char option1[50];
    char option2[50];
    char priority[50] = "L";
    char t2[50];
    char t1[50];
    int Q_time = 10;

    while(printf("$")) {
        //scanf("%[^\n]", temp);
        scanf("%s", temp);
        //printf("%s\n",temp);
        if(!strcmp(temp,"add")) {
            scanf("%s", task_name);
            fgets(temp, 100, stdin);
            sscanf(temp, "%s %s %s %s", option1, t1, option2, t2);
            if(!strcmp(option1, "-t")) {
                if(!strcmp(t1, "L")) {
                    Q_time = 20;
                } else {
                    Q_time = 10;
                }
            } else if(!strcmp(option1, "-p")) {
                if(!strcmp(t1, "H")) {
                    strcpy(priority, t1);
                } else if(!strcmp(t2, "L")) {
                    strcpy(priority, t2);
                }
            }
            if(!strcmp(option2, "-t")) {
                if(!strcmp(t2, "L")) {
                    Q_time = 20;
                } else {
                    Q_time = 10;
                }
            } else if(!strcmp(option2, "-p")) {
                if(!strcmp(t2, "H")) {
                    strcpy(priority, t2);
                } else if(!strcmp(t2, "L")) {
                    strcpy(priority, t2);
                }
            }
            //printf("Q_time = %d, priority = %s\n", Q_time, priority);
            add_readyq(task_name, Q_time, priority);
            strcpy(priority, "L");
            Q_time = 10;
            memset(option1,0, 50);
            memset(option2,0, 50);
            memset(t1,0, 50);
            memset(t2,0, 50);
        } else if(!strcmp(temp, "start")) {
            printf("Simulating...\n");
            if(run_q) {
                set_timer(Time/1000);
                swapcontext(&shell_mode, &current);
            } else {
                swapcontext(&shell_mode, &run_mode);
            }
        } else if(!strcmp(temp, "ps")) {

            Node *temp;
            temp = h_ready_q;
            while(temp) {
                //printf("time_quantum = %s\n", temp->quantum);
                printf("%d %s %s %lld %s %s\n", temp->pid, temp->name, temp->state, temp->total_queuing_time, temp->priority, temp->quantum);
                temp = temp->next;
                fflush(stdout);
            }
            temp = waiting_q;
            while(temp) {
                printf("%d %s %s %lld %s %s\n", temp->pid, temp->name, temp->state, temp->total_queuing_time, temp->priority, temp->quantum);
                temp = temp->next;
            }
            temp = run_q;
            while(temp) {
                printf("%d %s %s %lld %s %s\n", temp->pid, temp->name, temp->state, temp->total_queuing_time, temp->priority, temp->quantum);
                temp = temp->next;
            }
            temp = l_ready_q;
            while(temp) {
                printf("%d %s %s %lld %s %s\n", temp->pid, temp->name, temp->state, temp->total_queuing_time, temp->priority, temp->quantum);
                temp = temp->next;
            }
            temp = terminate_q;
            while(temp) {
                printf("%d %s %s %lld %s %s\n", temp->pid, temp->name, temp->state, temp->total_queuing_time, temp->priority, temp->quantum);
                temp = temp->next;
            }
        } else if(!strcmp(temp,"remove")) {
            int P;
            scanf("%d", &P);
            Node *temp;
            Node *front = NULL;
            Node *D = NULL;
            Node *t;
            t = h_ready_q;

            while(t) {
                temp = t;
                t = t->next;
                if(temp->pid == P) {
                    if(front == NULL) {
                        if(temp->next == NULL) {
                            h_ready_q = h_ready_e = NULL;
                            D = temp;
                        } else if(temp->next!=NULL) {
                            h_ready_q = temp->next;
                            D = temp;
                        }
                    } else {
                        if(temp->next == NULL) {
                            front->next = NULL;
                            h_ready_e = front;
                            D = temp;
                        } else if(temp->next != NULL) {
                            front->next = temp->next;
                            D = temp;
                        }
                    }
                    break;
                }
                front = temp;

            }
            if(D == NULL && run_q != NULL) {
                if(t->pid == P) {
                    D = t;
                    run_q = NULL;
                }
            } else if(D == NULL && run_q == NULL && waiting_q!=NULL) {
                //printf("!!!!!!");
                t = waiting_q;
                //while(t){
                //printf("!!!");
                Node *de_q;
                de_q = t;
                t = t->next;
                if(de_q->pid == P) {
                    //printf("waiting_q_name = %s, waiting_time =  %d\n", temp->name, temp->waiting_time);
                    if(de_q->next!=NULL) {
                        /*Not the end of waiting_q*/
                        if(de_q->front!=NULL) {
                            /*Not the front of waiting_q*/
                            de_q->front->next = de_q->next;
                            de_q->next->front = de_q->front;
                            de_q->front = NULL;
                            de_q->next = NULL;
                        } else { //first one
                            de_q->next->front = NULL;
                            waiting_q = de_q->next;
                            de_q->next = NULL;
                        }
                    } else {
                        if(de_q->front!=NULL) { //the last one
                            de_q->front->next = NULL;
                            waiting_e = de_q->front;
                            de_q->front = NULL;
                        } else {
                            waiting_q = NULL;
                            waiting_e = NULL;
                        }
                    }
                    /*if there is the only one node in the wainting_q do nothing*/
                    de_q->next = NULL;
                    D = de_q;
                    de_q = NULL;
                }
            }
            free(D);
        }
    }
}

int main()
{
    sa_flag = 0;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGALRM);
    sigaddset(&sa.sa_mask, SIGTSTP);
    sa.sa_flags = 0;
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    getcontext(&terminate);
    terminate.uc_stack.ss_sp = malloc(8192);
    terminate.uc_stack.ss_size = 8192;
    terminate.uc_stack.ss_flags = 0;
    terminate.uc_link = NULL;
    makecontext(&terminate, termination, 0);

    getcontext(&run_mode);
    run_mode.uc_stack.ss_sp = malloc(8192);
    run_mode.uc_stack.ss_size = 8192;
    run_mode.uc_stack.ss_flags = 0;
    makecontext(&run_mode, simulating, 0);

    shell();
    //printf("back");
    return 0;
}
