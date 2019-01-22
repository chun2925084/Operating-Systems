#include<linux/module.h>
#include<linux/skbuff.h>
#include<linux/init.h>
#include<linux/pid.h>
#include<linux/string.h>
#include<linux/kernel.h>
#include<linux/list.h>
#include<linux/sched.h>
#include<net/sock.h>
#include<net/netlink.h>


#define NETLINK_USER 31

struct sock *nl_sk = NULL;
char * dangle = NULL;

char *parent_pstree(int pid);
char *sibling_pstree(int pid);
char *children_pstree(int pid, int step, char* M);

static void hello_nl_recv_msg(struct sk_buff *skb)
{

    unsigned long int pid = 0;
    unsigned long int self_pid = 0;
    int msg_size = 0;
    int res = 0;
//    int i = 0;

    char option;
    char *str = (char *)kzalloc(1024*sizeof(char),GFP_KERNEL);
    char *msg = (char *)kzalloc(10*4096*sizeof(char),GFP_KERNEL);

    struct pid *pid_struct;
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh;
    struct task_struct *task;

    //printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);
    nlh = (struct nlmsghdr *)skb->data;
    //printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
    //printk("size = %ld\n",strlen((char *)nlmsg_data(nlh)));

    /*pid of sending process */
    self_pid = nlh->nlmsg_pid;

    /*input option & pid*/
    if(strlen((char *)nlmsg_data(nlh))==1) {

        sscanf(nlmsg_data(nlh),"%c",&option);
        if(option == 'c') {
            pid = 1;
        } else {
            pid = self_pid;
        }

        //printk("pid = %ld\n",pid);
        //printk("Option = %c\n", option);

        pid_struct = find_get_pid(pid);
        task = pid_task(pid_struct,PIDTYPE_PID);

    } else {

        sscanf(nlmsg_data(nlh),"%c %ld",&option, &pid);

        //printk("pid = %ld\n",pid);
        //printk("Option = %c\n", option);

        pid_struct = find_get_pid(pid);
        task = pid_task(pid_struct,PIDTYPE_PID);
        if(task == NULL) {
            option = 'N';
//		printk("option = %c\n",option);
        }
    }




    /* pstree */
    if(option == 'c') {

        char *str = (char *)kzalloc(32*sizeof(char),GFP_KERNEL);
        strcat(msg,task->comm);
        strcat(msg,"\0");
        sprintf(str,"(%d)\n",task->pid);
        strcat(msg,str);
        strcat(msg,"\0");
        msg = children_pstree(task->pid,0,msg);
        //printk("len = %ld\n",strlen(msg));
        kfree(str);
    } else if(option == 'p') {

        msg = parent_pstree(task->pid);
        if(!dangle) {
            kfree(dangle);
        }

    } else if(option == 's') {

        msg = sibling_pstree(task->pid);
        if(!dangle) {
            kfree(dangle);
        }
        //printk("end task = %d\n",task->pid);
    }

    msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, self_pid, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);
//    printk("nlmsg = %s\n",(char *)nlmsg_data(nlh));
    kfree(msg);
    res = nlmsg_unicast(nl_sk, skb_out, self_pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
    if(!msg) {
        kfree(msg);
    }
    if(!str) {
        kfree(str);
    }
}

static int __init hello_init(void)
{

    //nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg, NULL, THIS_MODULE);
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };
    //printk("Entering: %s\n", __FUNCTION__);
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        //printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{

    //printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}


char *parent_pstree(int pid)
{

    int count=0;
    int i=1;
    int j=0;

    int *parent_pid = (int*)kzalloc(32*sizeof(int),GFP_KERNEL);
    char *msg = (char *)kzalloc(512*sizeof(char),GFP_KERNEL);
    char *str = (char *)kzalloc(32*sizeof(char),GFP_KERNEL);

    struct pid *pid_struct = find_get_pid(pid);
    struct task_struct *T = pid_task(pid_struct,PIDTYPE_PID);

    char **parent_comm = NULL;
    int rows = 20;
    int cols = 100;
    parent_comm = (char **)kmalloc(rows*sizeof(char*),GFP_KERNEL);

    for(j=0; j<rows; ++j) {
        parent_comm[j] = (char *)kmalloc(cols*sizeof(char),GFP_KERNEL);
    }

    //strcat(msg,T->comm);
    //printk("msg = %s\n",msg);

    while(T->pid!=0) {
        //printk("task %s\n",T->comm);
        sprintf(parent_comm[count],"%s",T->comm);
        parent_pid[count] = T->pid;
        T = T->parent;
        ++count;
    }
    --count;
    while(count>=0) {
        /*connect the name of process*/
        strcat(msg,parent_comm[count]);
        strcat(msg,"\0");
        //printk("%s\n",msg);

        /*change the type of pid to char and connect*/
        sprintf(str,"(%d)\n",parent_pid[count]);
        //printk("%s\n",str);
        strcat(msg,str);
        strcat(msg,"\0");
        //printk("mes = %s\n",msg);

        for(j=0; j<i; j++) {
            strcat(msg,"    ");
            strcat(msg,"\0");
        }
        ++i;
        --count;
    }
    dangle = msg;
    if(!str) {
        kfree(str);
    }

    for(i=0; i<rows; i++) {
        if(!parent_comm[i]) {
            kfree(parent_comm[i]);
        }
    }

    if(!parent_comm) {
        kfree(parent_comm);
    }


    return msg;
}

char *sibling_pstree(int pid)
{

    char *msg = (char *)kzalloc(1024*sizeof(char),GFP_KERNEL);
    char *str = (char *)kzalloc(32*sizeof(char),GFP_KERNEL);
    struct list_head *ptr = NULL;
    struct task_struct *entry = NULL;
    struct pid *pid_struct = find_get_pid(pid);
    struct task_struct *T = pid_task(pid_struct,PIDTYPE_PID);
    list_for_each(ptr,&(T->sibling)) {
        entry = NULL;
        memset(str,0,strlen(str));


        entry = list_entry(ptr, struct task_struct, sibling);
        if(entry->pid == 0) {
            continue;
        }
        strcat(msg,entry->comm);
        strcat(msg,"\0");
        sprintf(str,"(%d)\n",entry->pid);
        strcat(msg,str);
        strcat(msg,"\0");
    }
    //		kfree(entry);
    if(!str) {
        kfree(str);
    }
    /*if(!ptr){
    	kfree(ptr);
    }*/
    dangle = msg;
    //printk("sibling_count = %d\n",sibling_count);
    return msg;
}

char *children_pstree(int pid, int step, char* M)
{

    struct pid *pid_struct = find_get_pid(pid);
    struct task_struct *T = pid_task(pid_struct,PIDTYPE_PID);
    char *str = (char *)kzalloc(128*sizeof(char),GFP_KERNEL);
    char *msg = (char *)kzalloc(1024*sizeof(char),GFP_KERNEL);
    struct list_head *ptr = NULL;
    struct task_struct *entry = NULL;
    int i = 0;
    msg = M;
    memset(str,0,strlen(str));
    ++step;
    list_for_each(ptr,&(T->children)) {
        entry = NULL;
        memset(str,0,strlen(str));
        entry = list_entry(ptr, struct task_struct, sibling);

        if(entry->pid == 0) {
            continue;
        }

        for(i = 0 ; i < step ; i++) {
            strcat(msg,"    ");
        }
        sprintf(M,"%s%s(%d)\n",msg,entry->comm,entry->pid);
        //strcat(M,entry->comm);
        //strcat(M,"\0");
        //sprintf(M,"%s(%d)\n",M,entry->pid);
        //strcat(M,str);
        //strcat(M,"\0");
        memset(str,0,strlen(str));
        //printk("Msg_len = %ld\n",strlen(M));
        //printk("comm = %s\n",entry->comm);
        //printk("Msg = %s\n",M);

        msg = children_pstree(entry->pid,step,msg);
    }
    kfree(str);
    dangle = M;
    return msg;
}


module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
