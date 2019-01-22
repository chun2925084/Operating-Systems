#include "client.h"

#define Buffer_size 14400

int PORT_NUM;
int sockfd = 0;
char* IP_NUM;
struct sockaddr_in info;

void build_socket();
void recursive_send(char* recv_msg, char curr_path[], char next_path[]);
void *runner(void *request);

int main(int argc, char *argv[])
{

    char *message = argv[2];
    char *receiveMessage = malloc(Buffer_size*sizeof(char));
    PORT_NUM = atoi(argv[6]);
    //IP_NUM = malloc(16*sizeof(char));
    IP_NUM = argv[4];
    char* request = malloc(2048*sizeof(char));

    sprintf(request, "GET %s HTTP/1.x\r\nHOST: %s:%d\r\n\r\n", message, IP_NUM, PORT_NUM);
    //printf("%s\n", request);


    /*Message sending*/
    build_socket();
    int length = send(sockfd, request, 100*sizeof(request),0);
    if(length == -1) {
        printf("send error\n");
    }
    recv(sockfd,receiveMessage, Buffer_size, 0);

    //printf("Close socket\n");
    close(sockfd);
    //printf("%s %d\n",message, length);
    mkdir("./output",0777);
    //printf("%s\n", receiveMessage);

    char * msg_for_next_path = malloc(2*strlen(message)*sizeof(char));
    recursive_send(receiveMessage, message, msg_for_next_path);

    return 0;
}

void recursive_send(char * recv_msg,char* curr_path,char* next_path)
{

    char* type = malloc(64*sizeof(char));
    char* status = malloc(64*sizeof(char));
    char* type_check = "directory";
    char* status_check = "OK";

    sscanf(recv_msg, "HTTP/1.x %*s %s", status);
    sscanf(recv_msg, "HTTP/1.x %*s %*s\r\nContent-type: %s", type);
    //printf("status = %s, type = %s\n", status, type);
    int is_dir =  (!strcmp(status,status_check) && !strcmp(type,type_check));
    free(type);
    free(status);
    if(is_dir) {
        char* file_dir = malloc(1024*sizeof(char));
        sscanf(recv_msg, "HTTP/1.x 200 OK\r\nContent-type: directory\r\nServer: httpserver/1.x\r\n\r\n%[^\n]", file_dir);
        char* pos = malloc(128*sizeof(char));
        sprintf(pos, "./output%s", curr_path);
        mkdir(pos,0777);
        free(pos);
        printf("%s\n", recv_msg);
        char *delim = " ";
        char* pch ;//=  malloc(64*sizeof(char));
        char* saveptr = file_dir;
        pch = __strtok_r(file_dir, delim,&saveptr);
        while(pch != NULL) {
            //next_path = malloc(128*sizeof(char));
            sprintf(next_path, "%s/%s", curr_path, pch);
            //printf("currpath = %s\n",curr_path);
            //printf("nextpath = %s\n",next_path);
            char* request = malloc(2048*sizeof(char));
            char* new_msg = malloc(Buffer_size*sizeof(char));
            sprintf(request, "GET %s HTTP/1.x\r\nHOST: %s:%d\r\n\r\n", next_path, IP_NUM, PORT_NUM);


            pch = __strtok_r(NULL,delim,&saveptr);

            pthread_t Thread;
            void *ret;
            pthread_create(&Thread, NULL, runner, request);
            pthread_join(Thread, &ret);
            //	build_socket();
            //	int length = send(sockfd, request, 100*sizeof(request),0);

            int ret_val = recv(sockfd,new_msg, Buffer_size, 0);
            if(ret_val == -1) {
                printf("receive error\n");
            }
            /*ret_val == 0 */
            //printf("Close socket 1\n");
            close(sockfd);


            char * msg_for_next_path = malloc(20+strlen(next_path)*sizeof(char));
            recursive_send(new_msg, next_path, msg_for_next_path);
            free(next_path);
            //free(msg_for_next_path);
        }
        free(file_dir);
    } else {
        printf("%s\n",recv_msg);
        char* pos2 = malloc(128*sizeof(char));
        sprintf(pos2, "./output%s", curr_path);
        //char* temp;// = malloc(1024*sizeof(char));
        char* write_in = malloc(14401*sizeof(char));
        sscanf(recv_msg,"%*s%*s%*s\r\n%*s\r\n%*s%*s%*s\r\n\r\n%14401c", write_in);
        //printf("write_in = %s\n", write_in);
        FILE *fptr;
        fptr = fopen(pos2,"w");
        if(!fptr) {
            printf("open file faili");

        } else {
            fprintf(fptr, "%s", write_in);
        }
        fclose(fptr);
        free(pos2);
        free(write_in);
        return ;

    }

}

void build_socket()
{

    /*creat socket*/
    int err;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("Fail to creat a socket\n");
    }
    /*connect socket*/
    memset(&info, 0, sizeof(info));
    //bzero(&info, sizeof(info));
    info.sin_family = AF_INET;

    /*local host*/
    info.sin_addr.s_addr = inet_addr(IP_NUM);
    info.sin_port = htons(PORT_NUM);

    err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err == -1) {
        printf("connect error");
        return ;
    }

    //printf("Open socket\n");
}

void *runner(void *request)
{


    build_socket();
    int length = send(sockfd, (char*)request, 100*sizeof(request),0);
    if(length == -1) {
        printf("send error\n");
    }
    pthread_exit(NULL);
}
