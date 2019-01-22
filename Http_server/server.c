#include "server.h"

#define file_content_size 14400

void* monitor(void *a);
void Rob_the_request();
char* Judge_type(char* file);
void List_Dir(char *D);
void bad_request();
void method_not_allowed();
void unsupported_media_type();

/*Queue*/
char* request_queue[1000];
int top = 0;
int num_of_request = 0;
pthread_mutex_t mutex;
int sockfd = 0,forClientSockfd = 0;
char message[] = {"Hi,this is server.\n"};
char *root;
char *file_content;

int main(int argc, char *argv[])

{
    /*information*/

    root = malloc(256*sizeof(char));
    //memset(root, 0, 256);
    root = argv[2];
    //if(*root == 47){
    //char* point =" .";
    //root = strcat(point, root);

    //}
    //printf("%s\n", root);
    int Thread_num = atoi(argv[6]);
    int PORT_NUM = atoi(argv[4]);
    file_content = malloc(file_content_size*sizeof(char));

    /* build socket */
    int buff_size = 2048;
    char *inputBuffer = (char *)malloc(buff_size*sizeof(char));
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }


    /*connect socket*/
    struct sockaddr_in serverInfo,clientInfo;
    memset(&serverInfo, 0, sizeof(serverInfo));

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(PORT_NUM);
    int n = bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    if(n!=0) {
        printf("Socket bind error");
    }
    //printf("%d\n", n);

    /*Multithread creating*/
    int i = 0;
    pthread_t Thread[Thread_num];
    pthread_mutex_init(&mutex, NULL);
    for(i = 0 ; i < Thread_num ; i++) {
        pthread_create(&Thread[i], NULL, monitor, NULL);
    }

    //    mkdir("./new dir",0700);


    while(1) {
        listen(sockfd,5);
        socklen_t addrlen = sizeof(clientInfo);
        forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
        //send(forClientSockfd,message,sizeof(message),0);
        //printf("buff_size*sizeof(char) = %ld\n",buff_size*sizeof(char));
        if(recv(forClientSockfd,inputBuffer,buff_size*sizeof(char),0)) {
            //printf("strlen(inputBuffer) = %ld\n",strlen(inputBuffer));
            request_queue[num_of_request] = malloc(strlen(inputBuffer));
            fflush(stdout);
            //printf("%s\n", inputBuffer);
            request_queue[num_of_request] = inputBuffer;
            //printf("Get:%s\n",request_queue[num_of_request]);
            ++num_of_request;
            //printf("top = %d\n num_of_request = %d\n",top, num_of_request);
        }
    }
    free(inputBuffer);
    free(file_content);
    return 0;
}

/*moniter the request_queue*/
void* monitor(void *a)
{
    //printf("enter the monitor!\n");
    while(1) {
        fflush(stdout);
        if(top < num_of_request) {
            Rob_the_request();
            //printf("in");
            //printf("got the request top = %d num = %d\n", top, num_of_request);
        }
    }

    pthread_exit(NULL);
}

void Rob_the_request()
{
    //printf("DDD");
    pthread_mutex_lock(&mutex);

    /*critical section*/
    char* target;// = malloc(strlen(request_queue[top]));
    target = request_queue[top];
    ++top;
    //printf("%s\n",target);
    pthread_mutex_unlock(&mutex);

    char *filename = malloc(1024*sizeof(char));
    char *header =  malloc(2048*sizeof(char));
    struct stat Dir;
    char *File_type = malloc(1024*sizeof(char));
    struct stat info;
    char *status;

    sscanf(target, "GET %s", filename);//get the name of file

    /*bad request_400*/
    if(*filename!=47) {
        //printf("target[0] = %d\n", *filename);
        bad_request();
        //printf("%s", file_content);
        send(forClientSockfd,file_content,2048*sizeof(file_content),0);
        free(filename);
        free(header);
        free(File_type);
        return;
    }
    /*method not allow_405*/
    else if(*target!=71) {
        method_not_allowed();
        //printf("%s", file_content);
        send(forClientSockfd,file_content,2048*sizeof(file_content),0);
        free(filename);
        free(header);
        free(File_type);
        return;
    }
    char *root2 = malloc(1024*sizeof(char));
    sprintf(root2, "%s%s", root, filename);//combine root and name
    sprintf(filename, "%s", root2);
    //free(root);
    free(root2);
    //printf("filename = %s\n", filename); //for test

    /*unsurpported media type*/
    stat(filename,&Dir);
    if(!S_ISDIR(Dir.st_mode)) {
        char *j_type = Judge_type(filename);
        if(*j_type == 48) {
            //printf("type = %s", j_type);
            unsupported_media_type();
            //printf("%s", file_content);
            send(forClientSockfd,file_content,2048*sizeof(file_content),0);
            free(header);
            free(File_type);
            return;
        }
    }


    /*step1 check if the input file is exist*/
    if(stat(filename,&info) != 0) {

        //printf("file name do not exist\n");
        char* header1 = "HTTP/1.x";
        status = "Not_Found\r\n";
        char* status_c = "404";
        //char*  status_c = strdup(status_code[2]);
        //char* status_c = malloc(sizeof(char));
        sprintf(header, "%s %s %s", header1, status_c, status);
        //printf("header = %s\n", header);
        char* content = "Content-type:\r\n";
        char* server = "Server: httpserver/1.x\r\n\r\n";
        //char* h_c = malloc(256*sizeof(char));
        //sprintf(h_c, "%s%s", header, content);
        sprintf(file_content, "%s%s%s", header, content, server);
        //free(h_c);
        //printf("file_content = \n%s\n", file_content);
    } else {
        //printf("exist\n");

        char *t = malloc(2048*sizeof(char));

        /*step2 check if a dir or not*/
        stat(filename,&Dir);
        if(S_ISDIR(Dir.st_mode)) {

            char* header1 = "HTTP/1.x";
            status = "OK\r\n";
            char* status_c = "200";
            char* file_type = "Content-type: directory\r\n";
            char* server = "Server: httpserver/1.x\r\n\r\n";
            sprintf(header, "%s %s %s", header1, status_c, status);
            sprintf(file_content, "%s%s%s", header, file_type, server);
            //free(File_type);
            //printf("%s", file_content);
            List_Dir(filename);
            //return;

        } else {

            FILE *fptr;
            fptr = fopen(filename, "r");
            if(fptr==NULL) {
                //printf("ERROR");
                free(t);
                free(header);
                free(File_type);
                return;
            }


            char* header1 = "HTTP/1.x";
            status = "OK\r\n";
            char* status_c = "200";
            char* temp = "Content-type: ";
            char* server = "Server: httpserver/1.x\r\n\r\n";
            sprintf(header, "%s %s %s", header1, status_c, status);
            int r_content_size = 14000;
            char* r_content = malloc(r_content_size*sizeof(char));
            memset(r_content, 0, r_content_size);
            /*File extention & File type*/
            char* type = Judge_type(filename);
            sprintf(File_type, "%s%s", temp, type);
            //printf("File_type = %s\n", File_type);
            //sprintf(file_content, "%s%s\r\n%s", header, File_type, server);
            //	printf("%s\n",file_content);

            while(fgets(t, 2048, fptr)!=NULL) {
                strcat(r_content,t);
                //sprintf(r_content,"%s%s", r_content, t);
                //sprintf(content, "%s%s", content, t);
            }
            //sprintf(file_content,"%s%s",file_content, r_content);
            sprintf(file_content, "%s%s\r\n%s%s", header, File_type, server, r_content);
            free(r_content);
            //free(header);
            //free(File_type);
            //printf("%s\n",file_content);
            fclose(fptr);

            /*FILE *fptrw;
            char *pos = malloc(128*sizeof(char));
            sprintf(pos, "./output/%s", filename);
            printf("pos = %s\n",filename);
            fptrw = fopen(pos, "w");
            if(fptrw==NULL){
            	printf("ERROR");
            }
            else{
            	fprintf(fptrw,"%s",content);

            }
            fclose(fptrw);*/
        }

        free(t);
        //free(File_type);
    }



    send(forClientSockfd,file_content,2048*sizeof(file_content),0);
    //	free(file_content);
    free(filename);
    free(File_type);
    free(header);
    return;

}


char* Judge_type(char* file)
{

    //printf("ext = %s\n", extensions[0].ext);
    int i = 0;
    int flag = 0;
    char* type = malloc(128*sizeof(char));
    char* temp = malloc(128*sizeof(char));
    sscanf(file,"./%s",temp);
    sscanf(temp,"%*[^.].%s",type);
    for(i=0; i<8; i++) {
        if(strcmp(type, extensions[i].ext) == 0) {
            temp = extensions[i].mime_type;
            flag = 1;
            //printf("temp = %s\n", temp);
        }

    }
    if(flag == 0) {

        temp = "0";

    }
    //printf("type = %s\n", type);
    free(type);
    return temp;
}

void List_Dir(char *D)
{

    DIR *d;
    struct dirent *dir;
    char* total_file = malloc(1024*sizeof(char));
    memset(total_file, 0, 1024);
    d = opendir(D);
    int i = 0;
    if(d) {
        while((dir = readdir(d)) != NULL) {
            if(strcmp(dir->d_name,".")==0 || strcmp(dir->d_name,"..")==0) {
                continue;
            }
            if(i==0) {
                strcat(total_file, dir->d_name);
                //sprintf(total_file, "%s%s", total_file, dir->d_name);
            } else {
                strcat(total_file, " ");
                strcat(total_file, dir->d_name);
                //sprintf(total_file, "%s %s", total_file, dir->d_name);
            }
            i++;
        }
        strcat(file_content, total_file);
        //sprintf(file_content, "%s%s", file_content, total_file);
        //printf("%s\n", file_content);
        closedir(d);
    }
    free(total_file);

}

void bad_request()
{

    char* header = malloc(1024*sizeof(char));
    char* header1 = "HTTP/1.x";
    char* status = "BAD_REQUEST\r\n";
    char* status_c = "400";
    char* content = "Content-type: \n";
    char* server = "Server: httpserver/1.x\r\n\r\n";
    sprintf(header, "%s %s %s", header1, status_c, status);
    sprintf(file_content, "%s%s%s", header, content, server);
    free(header);
}

void method_not_allowed()
{

    char* header = malloc(1024*sizeof(char));
    char* header1 = "HTTP/1.x";
    char* status = "METHOD_NOT_ALLOWED\r\n";
    char* status_c = "405";
    char* content = "Content-type: \n";
    char* server = "Server: httpserver/1.x\r\n\r\n";
    sprintf(header, "%s %s %s", header1, status_c, status);
    sprintf(file_content, "%s%s%s", header, content, server);
    free(header);

}

void unsupported_media_type()
{

    char* header = malloc(1024*sizeof(char));
    char* header1 = "HTTP/1.x";
    char* status = "UNSUPPORT_MEDIA_TYPE\r\n";
    char* status_c = "415";
    char* content = "Content-type: \n";
    char* server = "Server: httpserver/1.x\r\n\r\n";
    sprintf(header, "%s %s %s", header1, status_c, status);
    sprintf(file_content, "%s%s%s", header, content, server);
    free(header);

}
