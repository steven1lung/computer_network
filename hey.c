#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>

int main(){
    FILE *fp;
    fp=fopen("test_file.txt","w");

    //20mb ok
    //for(long int i=0;i<30000000;++i){
        //fprintf(fp,"%ld",i);
   //}


char *s = "hello, i am steven.";

for(int i=0;i<20000000;++i)
fprintf(fp,"%s",s);






    
    fprintf(fp,"%c",'\n');

    fclose(fp);
    return 0;
}