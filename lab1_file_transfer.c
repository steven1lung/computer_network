#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <math.h>
#include <arpa/inet.h>
#include <errno.h>

#define SIZE 2048
int count_u=0;
long int findSize(char* file_name){
    // opening the file in read mode
    FILE* fp = fopen(file_name, "r");
  
    // checking if the file exist or not
    if (fp == NULL) {
        printf("File Not Found!\n");
        return -1;
    }
  
    fseek(fp, 0L, SEEK_END);
  
    // calculating the size of the file
    long int res = ftell(fp);
  
    // closing the file
    fclose(fp);
  
    return res;
}

void print_time(){
    char curtime[128];
    time_t t=time(NULL);
    struct tm* ptm;
    t=time(NULL);
    ptm=localtime(&t);
    strftime(curtime,128,"%d-%b-%Y %H:%M:%S", ptm);
    printf("%s\n",curtime);
    return;
}

void send_file(FILE* fp,int sockfd,int long file_size){
    char data[SIZE] = {0};
    long int total_transfer =0;
    int printed[5]={0};

    //test
    unsigned int out_bytes=0;
    int count=0;
    char* size ;
    //sprintf(size,"%ld",file_size);
   // strcpy(data,size);
    //send(sockfd,data,SIZE,0);
  


    while(fgets(data,SIZE,fp)!=NULL){
        
        int n=send(sockfd,data,sizeof(data),0); 
        //int n=write(sockfd,data,SIZE);
        count++;
        total_transfer+=n;
        
        if(total_transfer>=0 && !printed[0]) {
            printf("%d%c ",0,'%');
            print_time();
            printed[0]=1;
        }
        else if (!printed[1] && total_transfer>=file_size/4  ){
            printf("%d%c ",25,'%');
            print_time();
            printed[1]=1;
        }
        else if (!printed[2] && total_transfer>=file_size/2  ){
            printf("%d%c ",50,'%');
            print_time();
            printed[2]=1;
        }
        else if (!printed[3] && total_transfer>=file_size/1.3  ){
            printf("%d%c ",75,'%');
            print_time();
            printed[3]=1;
        }
        
        out_bytes+=n;
        //printf("Response sent (%d bytes).%d\n", n,out_bytes);
        bzero(data,SIZE);
        usleep(1);
        count_u++;
        
    }
    printf("%d%c ",100,'%');

    print_time();
    fclose(fp);
}

void tcp_client(const char* ip,const char* port,const char* filename,long int file_size){
    //printf("IP is %s\n",ip);
    // variables
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[SIZE];
    FILE *fp;

    //initialize
    fp=fopen(filename,"r");
    if(fp==NULL){
        perror("error in reading file\n");
        exit(1);
    }

    portno=atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) perror("ERROR opening socket");
    //printf("socket created\n");
    server = gethostbyname(ip);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    n=connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(n<0){
        perror("ERROR connecting\n");
        exit(1);
    }
    //printf("connected to server\n");

    //start transfer
    send_file(fp,sockfd,file_size);
    
    //printf("data sent successfully\n");

    //printf("socket closed\n");


    return;
}

void write_file(const int sockfd,long int file_size){
    int n,count=0;
    long int total_transfer=0;
    FILE* fp;
    char* file = "file_transferred.txt";
    char buffer[SIZE];
    int printed[5]={0};
    //int long size;
    //char* str;
    fp = fopen(file,"w");
    if(fp==NULL) {
        perror("error in creating file\n");
        exit(1);
    }

    //n=recv(sockfd,buffer,SIZE,0);
    //sprintf(str,"%s",buffer);
    //size=atoi(str);
    //bzero(buffer,SIZE);
    
    while(1){
        
        n=recv(sockfd,buffer,SIZE,0);
        if(n<=0) break;
        count++;
        total_transfer+=n;
        //printf("%d\n",n);
        
        //printf("%ld %ld\n",file_size,total_transfer);
        if(!printed[0]&&total_transfer>=0) {
            printf("%d%c ",0,'%');
            print_time();
            printed[0]=1;
        }
        else if (!printed[1] && total_transfer>=file_size/4  ){
            printf("%d%c ",25,'%');
            print_time();
            printed[1]=1;
        }
        else if (!printed[2] && total_transfer>=file_size/2  ){
            printf("%d%c ",50,'%');
            print_time();
            printed[2]=1;
        }
        else if (!printed[3] && total_transfer>=file_size/1.3  ){
            printf("%d%c ",75,'%');
            print_time();
            printed[3]=1;
        }

        
        fprintf(fp,"%s",buffer);
        bzero(buffer,SIZE);
        //printf("Response recieved (%d bytes).%d\n", n,count);
    }
    fprintf(fp,"%s",buffer);




    printf("%d%c ",100,'%');
    print_time();
    //printf("Packet Loss Rate : %lf%%",(double)total_transfer/size);
    fclose(fp);
    return;

}

void tcp_host(const char* ip,const char* port,const char* filename,long int file_size){
    //printf("%s\n",ip);
    
    //variables
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    //initialize
    //printf("IP is %s\n",ip);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) perror("ERROR opening socket");
    //printf("socket created\n");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(port);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) perror("ERROR on binding");
    //printf("bind success\n");


    //start listen
    n=listen(sockfd,10);
    if(n!=0){
        perror("error in listening\n");
        exit(1);
    }

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
    
    //write in
    write_file(newsockfd,file_size);
    //printf("data written\n");


    
    return;
}

void send_file_data(FILE* fp,int sockfd,struct sockaddr_in addr,int long file_size){
    int n,count=0,total_transfer=0;
    int printed[5]={0};
    char buffer[SIZE];
    fp=fopen("test_file.txt","r");
    if(fp==NULL) printf("error opening file\n");

    while(fgets(buffer,SIZE,fp)!=NULL){
    
    n=sendto(sockfd,buffer,SIZE,0,(struct sockaddr*)&addr,sizeof(addr));

    count++;
    total_transfer+=n;
    if((SIZE & SIZE)==1)usleep(1);
    if(total_transfer>=0 && !printed[0]) {
        printf("%d%c ",0,'%');
        print_time();
        printed[0]=1;
    }
    else if (!printed[1] && total_transfer>=file_size/4  ){
        printf("%d%c ",25,'%');
        print_time();
        printed[1]=1;
    }
    else if (!printed[2] && total_transfer>=file_size/2  ){
        printf("%d%c ",50,'%');
        print_time();
        printed[2]=1;
    }
    else if (!printed[3] && total_transfer>=file_size/1.3  ){
        printf("%d%c ",75,'%');
        print_time();
        printed[3]=1;
    }


    usleep(2);
    if(n==-1) {
        printf("error sending data\n");
        exit(1);
    }
    if((SIZE & SIZE)==1)usleep(1000);
    bzero(buffer,SIZE);


    }
    strcpy(buffer,"END");
    sendto(sockfd,buffer,SIZE,0,(struct  sockaddr*)&addr,sizeof(addr));
    printf("%d%c ",100,'%');
    print_time();
    fclose(fp);
    return;
}

void write_file_udp(int sockfd,struct sockaddr_in addr,int long file_size){
    FILE* fp=fopen("file_transfered_udp.txt","w");
    char buffer[SIZE]={0};
    int n,count=0,total_transfer=0;
    int printed[5]={0};
    socklen_t addr_size= sizeof(addr);
    
    
    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    while(1){

        peerlen = sizeof(peeraddr);
        addr_size= sizeof(addr);
        //n=recvfrom(sockfd,buffer,SIZE,0,(struct sockaddr*)&addr,(socklen_t*)addr_size);
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);

        count++;
        total_transfer+=n;
        //printf("%d\n",n);
        
        //printf("%ld %ld\n",file_size,total_transfer);
        if(total_transfer>=0 && !printed[0]) {
            printf("%d%c ",0,'%');
            print_time();
            printed[0]=1;
        }
        else if (!printed[1] && total_transfer>=file_size/4  ){
            printf("%d%c ",25,'%');
            print_time();
            printed[1]=1;
        }
        else if (!printed[2] && total_transfer>=file_size/2  ){
            printf("%d%c ",50,'%');
            print_time();
            printed[2]=1;
        }
        else if (!printed[3] && total_transfer>=file_size/1.3  ){
            printf("%d%c ",75,'%');
            print_time();
            printed[3]=1;
        }

        if(strcmp(buffer,"END")==0){
            break;
        }
        //printf("data get\n",buffer);

        fprintf(fp,"%s",buffer);
        bzero(buffer,SIZE);
    }   
    printf("%d%c ",100,'%');
    print_time();
    fclose(fp);
    close(sockfd);
    return;
}

void udp_host(const char* ip,const char* port,const char* filename,long int file_size){
    //initialize
    int sock,newsock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("socket error\n");
        exit(1);
    }
    char buffer[SIZE];
    struct sockaddr_in servaddr,cliaddr;
    socklen_t clilen;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        printf("bind eerror\n");
        exit(1);
    }
    //printf("UDP server started\n");
  


    write_file_udp(sock,cliaddr,file_size);
    
    close(sock);
    

    return;
}

void udp_client(const char* ip,const char* port,const char* filename,long int file_size){
    //initialize
    
    int sock; 
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        printf("socket error\n");
        exit(1);
    }
    FILE* fp;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(port));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    fp=fopen(filename,"r");

    send_file_data(fp,sock,servaddr,file_size);
   //getchar();
    return;
}

int main(int argc,char * argv[]){
    /*
    argv[1]  tcp or udp
    argv[2]  send or recv
    argv[3]  ip
    argv[4]  port
    argv[5]  filename
    */

    argv[5]="test_file.txt";
    struct stat st;
    stat(argv[5], &st);
    long int file_size = st.st_size;

    clock_t begin = clock(); //begin timing
    //long int file_size=findSize(argv[5]);
    
    if(strcmp(argv[1],"tcp")==0){
        //heyhey
        if(strcmp(argv[2],"send")==0) tcp_client(argv[3],argv[4],argv[5],file_size);
        else if(strcmp(argv[2],"recv")==0) tcp_host(argv[3],argv[4],argv[5],file_size);
        else printf("please enter send or recv ! !\n");
    }
    else if (strcmp(argv[1],"udp")==0){
        
        if(strcmp(argv[2],"send")==0) udp_client(argv[3],argv[4],argv[5],file_size);
        else if(strcmp(argv[2],"recv")==0) udp_host(argv[3],argv[4],argv[5],file_size);
        else printf("please enter send or recv ! !\n");
    }
    else printf("please enter udp or tcp ! !\n");
    clock_t end = clock(); //end timing
    
    //printf("%s ended\n",argv[2]);

    //print result
    
    printf("Total Tranfer Time : %lf ms\n",(double)(end - begin) / CLOCKS_PER_SEC*1000.0-count_u/10000);
    printf("File Size : %lf MB\n",file_size/1000.0/1000.0);
    if(strcmp(argv[1],"udp")==0){
        struct stat reality;
        stat("file_transfered_udp.txt", &reality);
        long int file_size_udp = reality.st_size;
        if(strcmp(argv[2],"recv")==0)
            printf("Packet Loss Rate : %lf%%\n",(double)file_size_udp/file_size);
    }
    return 0;
}