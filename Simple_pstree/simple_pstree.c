#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/types.h>
#include<linux/netlink.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 4096 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;
void* MSG;

int main(int argc, char *argv[])
{
    char *token;// = NULL;
    char *temp;// = NULL;
    int len = 0;
    int i = 0;
    int count = 0;
    char *target = (char *)malloc((10)*sizeof(char));
    char *blank = (char *)malloc(sizeof(char));
    if(argv[1] != NULL) {
        temp = strdup((const char*)argv[1]);
        token = temp + 1;
        len = strlen(token);
        memset(blank,32,1);
        target[0] = token[0];
        if(len>1) {
            target[1] = blank[0];
            int i=1;
            for(i = 1; i < len ; i++) {
                target[i+1] = token[i];
            }
        }
    } else if(argv[1] == NULL) {
        target[0] = 'c';
        target[1] = '1';
    }
    //printf("%s\n",target);

    if(target[0]!='s' && target[0]!= 'p' && target[0]!= 'c') {

        free(target);
        free(blank);
        return 0;

    }

    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        free(target);
        free(blank);
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */
    //printf("%d\n",src_addr.nl_pid);

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;


    strcpy(NLMSG_DATA(nlh), target);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);
    // printf("Waiting for message from kernel\n");
    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    //printf("Received message payload: %p\n", NLMSG_DATA(nlh));
    //printf("nlh = %s\n",(char *)NLMSG_DATA(nlh));
    for(i=0; i<strlen(target); i++) {
        if(*(char *)((NLMSG_DATA(nlh))+i) == *(char *)(target+i)) {
            count++;
        }
    }

    //printf("len = %ld\n",strlen((char *)NLMSG_DATA(nlh)));
    if(count!=strlen(target)) {
        printf("%s\n", (char *)NLMSG_DATA(nlh));
    }
    fflush(stdout);
    free(target);
    free(blank);


}

