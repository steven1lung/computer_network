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

#define SIZE 2048  //set buffer size to 2048
int count_u=0;     //count how many msecs was in the while loop
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
    strftime(curtime,128,"%d-%b-%Y %H:%M:%S", ptm);   //print the time in date-month-year time
    printf("%s\n",curtime);
    return;
}

void send_file(FILE* fp,int sockfd,int long file_size){
    //this is the function the client will call
    //send file to server
    char data[SIZE] = {0};       //set buffer size
    long int total_transfer =0;  //know how many bytes that have been transferred
    int printed[5]={0};          //print 0% 25% 50% 75% once only since im using > as a condition

 





    while(fgets(data,SIZE,fp)!=NULL){
        
        int n=send(sockfd,data,sizeof(data),0); //send to socket
 

        total_transfer+=n;     //add n to total_transfer since send will return how many bytes that have neen sent
        
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
        
        bzero(data,SIZE);    //claer the buffer
        usleep(1);         //wait for 1 msec because sending too fast will cause error
        count_u++;         
        
    }
    printf("%d%c ",100,'%');

    print_time();
    fclose(fp);
}

void tcp_client(const char* ip,const char* port,const char* filename,long int file_size){
    // variables
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[SIZE];
    FILE *fp;

    //initialize
    fp=fopen(filename,"r");                       //client open file that will be sent
    if(fp==NULL){
        perror("error in reading file\n");
        exit(1);
    }

    portno=atoi(port);                               //change the port num to int
    sockfd = socket(AF_INET, SOCK_STREAM, 0);        //set the socket
    if (sockfd < 0) perror("ERROR opening socket");  

    server = gethostbyname(ip);                      //set IP
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));       //set buffer to 0
    serv_addr.sin_family = AF_INET;                      //setup server     
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);              //set server port to the port get from cmd line

    n=connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));  //connect socket
    if(n<0){
        perror("ERROR connecting\n");
        exit(1);
    }
  

    //start transfer
    send_file(fp,sockfd,file_size);
    return;
}

void write_file(const int sockfd,long int file_size){
    int n;                                            
    long int total_transfer=0;
    FILE* fp;                                     //server write in the file sent
    char* file = "file_transferred.txt";
    char buffer[SIZE];                             //set buffer size to 2048
    int printed[5]={0};                           //for transfer log
    fp = fopen(file,"w");
    if(fp==NULL) {
        perror("error in creating file\n");
        exit(1);
    }

    
    while(1){
        
        n=recv(sockfd,buffer,SIZE,0);             //recieve the data sent from client
        if(n<=0) break;                            //if 0 means no more data are being transferred            
        total_transfer+=n;                       //know how many bytes that have been recieved
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

        
        fprintf(fp,"%s",buffer);              //write in to file pointer
        bzero(buffer,SIZE);                  //clear buffer
    }
            




    printf("%d%c ",100,'%');
    print_time();

    fclose(fp);
    return;

}

void tcp_host(const char* ip,const char* port,const char* filename,long int file_size){
    //variables
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    //initialize
    sockfd = socket(AF_INET, SOCK_STREAM, 0);                       //setup socket
    if (sockfd < 0) perror("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));            //set serv_addr to 0
    portno = atoi(port);                                        //set port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) perror("ERROR on binding");  //bind the socket


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



    
    return;
}

void send_file_data(FILE* fp,int sockfd,struct sockaddr_in addr,int long file_size){
    //variables
    int n,total_transfer=0;                  
    int printed[5]={0};
    char buffer[SIZE];  
    fp=fopen("test_file.txt","r");               //open the file that will be sent
    if(fp==NULL) printf("error opening file\n");

    while(fgets(buffer,SIZE,fp)!=NULL){                            //read file from fp and save into buffer
    
    n=sendto(sockfd,buffer,SIZE,0,(struct sockaddr*)&addr,sizeof(addr));   //send buffer to server socket

    total_transfer+=n;                                  //add n bytes to total_transfer
    usleep(1);                                        //wait for 1 msec because sending too fause causes error 
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


    
    if(n==-1) {
        printf("error sending data\n");
        exit(1);
    }
    
    bzero(buffer,SIZE);                    //clear buffer
    }
    strcpy(buffer,"END");                    //pu string "END" to buffer
    sendto(sockfd,buffer,SIZE,0,(struct  sockaddr*)&addr,sizeof(addr));     //send "END" to client socket to let them know transfer end
    printf("%d%c ",100,'%');
    print_time();
    fclose(fp);
    return;
}

void write_file_udp(int sockfd,struct sockaddr_in addr,int long file_size){
    //variables
    FILE* fp=fopen("file_transfered_udp.txt","w");             //write in to file_tranferred_udp when recieve
    char buffer[SIZE]={0};             
    int n,total_transfer=0;
    int printed[5]={0};
    socklen_t addr_size= sizeof(addr);
    
    
    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    while(1){

        peerlen = sizeof(peeraddr);
        addr_size= sizeof(addr);
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);      //recieve data from client socket and store in buffer

        
        total_transfer+=n;                               //know how many bytes tranferred so far

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


        fprintf(fp,"%s",buffer);                   //fp write in buffer data
        bzero(buffer,SIZE);                     //clear buffer
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
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){         //create socket
        printf("socket error\n");
        exit(1);
    }
    //variables
    char buffer[SIZE];                       
    struct sockaddr_in servaddr,cliaddr;     
    socklen_t clilen;
    memset(&servaddr, 0, sizeof(servaddr));            //set servaddr to 0           
    servaddr.sin_family = AF_INET;                     //setup servaddr
    servaddr.sin_port = htons(atoi(port));            //set port
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);       //set IP

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){   //bind socket
        printf("bind eerror\n");
        exit(1);
    }
  
  


    write_file_udp(sock,cliaddr,file_size);
    
    close(sock);
    

    return;
}

void udp_client(const char* ip,const char* port,const char* filename,long int file_size){
    //initialize
    
    int sock; 
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){          //setup socket
        printf("socket error\n");
        exit(1);
    }
    FILE* fp;                                   //file pointer for sending the data

    //vairables
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));        //set servaddr to 0
    servaddr.sin_family = AF_INET;                  //setup servaddr
    servaddr.sin_port = htons(atoi(port));           //set port
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);       //set IP
 
    fp=fopen(filename,"r");                   //open file

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
    long int file_size = st.st_size;        //the test_file size

    clock_t begin = clock(); //begin timing
    
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
    


    //print result
    
    printf("Total Tranfer Time : %lf ms\n",(double)(end - begin) / CLOCKS_PER_SEC*1000.0-count_u/10000);  //print tranfer time
    printf("File Size : %lf MB\n",file_size/1000.0/1000.0);               //print file size
    if(strcmp(argv[1],"udp")==0){
        struct stat reality;
        stat("file_transfered_udp.txt", &reality);
        long int file_size_udp = reality.st_size;               //get transferred file size
        if(strcmp(argv[2],"recv")==0)
            printf("Packet Loss Rate : %lf%%\n",(double)file_size_udp/file_size);   //print packet loss rate
    }
    return 0;
}