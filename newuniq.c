#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[]){

char past[128];

int u=0;
int lcount=0;
FILE *f = fopen(argv[1],"r");
char line[128];
while (fgets(line,128,f)){


    lcount++;

    if (lcount==1){

        u++;
        strcpy(past,line);
    }
    else{

        if(strcmp(past,line)!=0){


            u++;
        strcpy(past,line);

        }

    }

}


fclose(f);


char uniq_lines[u][128];
int count[u];
char past2[128];
char line2[128];
char temp[128];

int j=0;
int k=0;
int s =0;
int ss=0;
FILE *f2 = fopen(argv[1],"r");
while (fgets(line2,128,f2)){

j++;
if (j==1){

        k++;
        strcpy(past2,line2);
        strcpy(uniq_lines[0],line2);

    }
    else{

        if(strcmp(past2,line2)!=0){

        strcpy(uniq_lines[k],line2);
      
        count[k-1]=s;
        
        
        
        s=0;    
        k++;
        strcpy(past2,line2);
        }

    }
   
s++;


}

fclose(f2);
count[k-1]=s;


if (argv[2] !=NULL){

if(strcmp(argv[2],"-c")==0 || strcmp(argv[2],"-count")==0){

for (int y=0; y< u; y++){
    printf("%d %s",count[y],uniq_lines[y]);

}


}
else{
printf("Illegal argument: %s\n",argv[2]);


}
}
else{

for (int y=0; y< u; y++){
    printf("%s",uniq_lines[y]);

}



}







return 0;
}




