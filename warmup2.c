#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <signal.h>

#include "my402list.h"
#include "cs402.h"

extern int errno;
long int B=10,P=3;
long int num=20;
double lambda=1.0,mu=0.35,r=1.5;
long int tokens=0,packets=0, served=0, drop_tok=0, drop_pac=0;
char *filename=NULL;
double tot_time_q1=0.0,tot_time_q2=0.0;
double tot_time_s1=0.0,tot_time_s2=0.0;
double tot_time_sys=0.0, tot_time_lambda=0.0;
double tot_sys_sqr=0.0,tot_ser_s1=0.0,tot_ser_s2=0.0;

sigset_t set;
int die=0;

My402List Q1,Q2;
long int bucket=0;
struct timeval init_time;
double timestr;
double init_time_d;
int trace_driven=0;

pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv=PTHREAD_COND_INITIALIZER;

struct packet{
 long int pac_no;
 long int pac_P;
 double pac_mu;
 double pac_lambda;
 double entry_time;
 double depart_time;
 double Q1_entry;
 double Q1_depart;
 double Q2_entry;
 double Q2_depart;
};
 
double timestamp(struct timeval time){
 return ((time.tv_sec)*1000)+((time.tv_usec)/1000.00); 
}

double absol(double x){
 if(x>0.0)
  return x;
 else
  return -x;
}
 
void* pop_buck(){
 sigprocmask(SIG_BLOCK, &set,0);
 struct timeval timer;
 double timestr;
 
 while(1){
  usleep((1/r)*1000000.00F); //takes in milliseconds
 
  pthread_mutex_lock(&m); 
  if(die==1){
   pthread_mutex_unlock(&m);
   //printf("token thread:CntrlC pressed\n");
   pthread_exit((void *)1);
  }

  tokens++;
  
  if(My402ListEmpty(&Q1) && packets==num){
   pthread_mutex_unlock(&m);
   return NULL;
  }

  gettimeofday(&timer,NULL);
  timestr=timestamp(timer);
  //if(served<num){
  if(bucket<B){
   bucket++;
   if(bucket<=1){
    printf("%012.3lfms:token t%ld arrives,token bucket now has %ld token\n",timestr-init_time_d,tokens,bucket);
   
   }
   else{
    printf("%012.3lfms:token t%ld arrives,token bucket now has %ld tokens\n",timestr-init_time_d,tokens,bucket);
   } 
  }
  else{
   printf("%012.3lfms:token t%ld arrives, dropped\n",timestr-init_time_d,tokens);
   drop_tok++;
  }
 
 //check if Q1 is eligible for transmission
  if(!My402ListEmpty(&Q1)){
   My402ListElem *new_packet;
   new_packet=My402ListFirst(&Q1);
   struct packet *new_pack=(struct packet *)new_packet->obj;
  
   if(bucket>=new_pack->pac_P){

    My402ListUnlink(&Q1,new_packet);
   
    gettimeofday(&timer,NULL);
    timestr=timestamp(timer);
    new_pack->Q1_depart=timestr-init_time_d;
   
    gettimeofday(&timer,NULL);
    timestr=timestamp(timer);
    if(bucket<=1){
     printf("%012.3lfms:p%ld leaves Q1, time in Q1 = %lfms, token bucket now has %ld token\n",timestr-init_time_d,new_pack->pac_no,absol(new_pack->Q1_depart-new_pack->Q1_entry),bucket);
     tot_time_q1+=absol(new_pack->Q1_depart-new_pack->Q1_entry);
    }
    else{
     printf("%012.3lfms:p%ld leaves Q1, time in Q1 = %lfms, token bucket now has %ld tokens\n",timestr-init_time_d,new_pack->pac_no,absol(new_pack->Q1_depart-new_pack->Q1_entry),bucket);
     tot_time_q1+=absol(new_pack->Q1_depart-new_pack->Q1_entry);    
    }
   
    My402ListAppend(&Q2, (void *)new_pack);
    gettimeofday(&timer,NULL);
    timestr=timestamp(timer);
    printf("%012.3lfms:p%ld enters Q2\n",timestr-init_time_d,new_pack->pac_no);
    new_pack->Q2_entry=timestr-init_time_d;
   
   //wake up the servers;
    pthread_cond_broadcast(&cv);
   }
  }
 pthread_mutex_unlock(&m);
 }
 return NULL;
}

void* pop_q1(){
 sigprocmask(SIG_BLOCK, &set,0);
 struct timeval timer;
 double timestr;
 char *line;
 char *param_tok;
 FILE *tptr;

 if(trace_driven==1){
  line=(char *)malloc(60*sizeof(char));
  tptr= fopen(filename,"r");
  fgets(line,60,tptr);
 }
 while(1){

  if(trace_driven==1){
   fgets(line,60,tptr);
   param_tok=strtok(line,"	 ");  
   lambda=atof(param_tok);
   if(lambda<0.1)
    lambda=0.1;
  }
  struct packet *new_pack;
  new_pack=(struct packet *)malloc(sizeof(struct packet));
  
  new_pack->pac_lambda=lambda;
  if(trace_driven==1){
   param_tok=strtok(NULL,"	 ");
  }
  usleep((1/(new_pack->pac_lambda))*1000000.00F);
  
  pthread_mutex_lock(&m);
  if(die==1){
   pthread_mutex_unlock(&m);
   pthread_exit((void *)1);
  }
  if(packets>=num){
   pthread_mutex_unlock(&m);
   return NULL;
  }
 
  tot_time_lambda+=new_pack->pac_lambda;
  packets++;
  new_pack->pac_no=packets;
 
  if(trace_driven==1){
   P=atoi(param_tok);
  }
  new_pack->pac_P=P;
 
  gettimeofday(&timer,NULL);
  timestr=timestamp(timer);
  if(new_pack->pac_P<=B){
  printf("%012.3lfms:p%ld arrives, needs %ld tokens, inter-arrival time=%lfms\n",timestr-init_time_d, new_pack->pac_no,new_pack->pac_P,new_pack->pac_lambda);
  }
  else{
   printf("%012.3lfms:p%ld arrives, needs %ld tokens, inter-arrival time=%lfms, dropped\n",timestr-init_time_d, new_pack->pac_no,new_pack->pac_P,new_pack->pac_lambda);
   drop_pac++;
   served=new_pack->pac_no;
   pthread_mutex_unlock(&m);
   continue;
  }
  
  new_pack->entry_time=timestr-init_time_d;
  if(trace_driven==1){
   param_tok=strtok(NULL,"	 ");
   mu=atof(param_tok);
   if(mu<0.1)
    mu=0.1;
  }
  new_pack->pac_mu=mu;

  //pthread_mutex_lock(&m); 
  
  if(!My402ListEmpty(&Q1)){
   
   My402ListAppend(&Q1, (void *)new_pack);
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   printf("%012.3lfms:p%ld enters Q1\n",timestr-init_time_d, new_pack->pac_no);
   new_pack->Q1_entry=timestr-init_time_d;
   
  }
  else if(bucket>=new_pack->pac_P){
   bucket-=new_pack->pac_P;
   
   My402ListAppend(&Q1, (void *)new_pack);
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   printf("%012.3lfms:p%ld enters Q1\n",timestr-init_time_d, new_pack->pac_no);
   new_pack->Q1_entry=timestr-init_time_d;
   
   My402ListElem *new_packet;
   new_packet=My402ListFirst(&Q1);
   struct packet *curr_pack=(struct packet *)new_packet->obj;
   My402ListUnlink(&Q1,new_packet);//check

   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   curr_pack->Q1_depart=timestr-init_time_d;
   
   
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   if(bucket<=1){
    printf("%012.3lfms:p%ld leaves Q1, time in Q1 = %lfms, token bucket now has %ld token\n",timestr-init_time_d,curr_pack->pac_no,absol(curr_pack->Q1_depart-curr_pack->Q1_entry),bucket);
    tot_time_q1+=absol(curr_pack->Q1_depart-curr_pack->Q1_entry);
   }
   else{
    printf("%012.3lfms:p%ld leaves Q1, time in Q1 = %lfms, token bucket now has %ld tokens\n",timestr,curr_pack->pac_no,absol(curr_pack->Q1_depart-curr_pack->Q1_entry),bucket); 
    tot_time_q1+=absol(curr_pack->Q1_depart-curr_pack->Q1_entry);    
   }

   My402ListAppend(&Q2, (void *)curr_pack);
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   printf("%012.3lfms:p%ld enters Q2\n",timestr-init_time_d,curr_pack->pac_no);
   curr_pack->Q2_entry=timestr-init_time_d;
   
   //wake up the servers;
   pthread_cond_broadcast(&cv);
   
  }
  else{
   My402ListAppend(&Q1, (void *)new_pack);
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   printf("%012.3lfms:p%ld enters Q1\n",timestr-init_time_d, new_pack->pac_no);
   new_pack->Q1_entry=timestr-init_time_d;
  }
  pthread_mutex_unlock(&m);
 }
 return NULL;
}

void * s1_service(){
 sigprocmask(SIG_BLOCK, &set,0);
 struct timeval timer;
 double timestr;
 
 while(1){

  pthread_cond_wait(&cv, &m);

  if(served==num && My402ListEmpty(&Q1)){
   pthread_mutex_unlock(&m);
   return NULL;
  }  

  if(My402ListEmpty(&Q2)){
   pthread_mutex_unlock(&m);
   continue;
  }
  
  else{
   My402ListElem *curr_packet=My402ListFirst(&Q2);
   struct packet *curr_pack=(struct packet *)curr_packet->obj;
   My402ListUnlink(&Q2,(void *)curr_packet);
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   curr_pack->Q2_depart=timestr-init_time_d;
   printf("%012.3lfms:p%ld leaves Q2, time in Q2 = %lfms\n",timestr-init_time_d,curr_pack->pac_no,absol(curr_pack->Q2_depart-curr_pack->Q2_entry));
   tot_time_q2+=absol(curr_pack->Q2_depart-curr_pack->Q2_entry);
   
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   printf("%012.3lfms:p%ld begins service at S1, requesting %lfms of service\n", timestr-init_time_d,curr_pack->pac_no, curr_pack->pac_mu);

   usleep((1/curr_pack->pac_mu)*1000000.00F);

   if(die==1){
    pthread_mutex_unlock(&m);
    //printf("s1:CntrlC pressed\n");
    pthread_exit((void *)1);
   }
 
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   curr_pack->depart_time=timestr-init_time_d;

   printf("%012.3lfms:p%ld departs from S1, service time=%lfms, time in system = %lfms\n", timestr-init_time_d,curr_pack->pac_no, curr_pack->pac_mu,absol(curr_pack->entry_time-curr_pack->depart_time));
   tot_ser_s1+=(curr_pack->pac_mu);
   tot_time_s1+=absol(curr_pack->Q2_depart-curr_pack->depart_time);
   tot_time_sys+=absol(curr_pack->entry_time-curr_pack->depart_time);
   tot_sys_sqr=absol(curr_pack->entry_time-curr_pack->depart_time)*absol(curr_pack->entry_time-curr_pack->depart_time);
   served=curr_pack->pac_no;
  }
  
 }
 pthread_mutex_unlock(&m);
 return NULL;
}

void * s2_service(){
 sigprocmask(SIG_BLOCK, &set,0);
 struct timeval timer;
 double timestr;

 while(1){
 
  pthread_cond_wait(&cv, &m);
  
  if(served==num && My402ListEmpty(&Q1)){
   pthread_mutex_unlock(&m);
   return NULL;
  }

  if(My402ListEmpty(&Q2)){
   pthread_mutex_unlock(&m);
   continue;
  }
  
  else{
   My402ListElem *curr_packet=My402ListFirst(&Q2);
   struct packet *curr_pack=(struct packet *)curr_packet->obj;
   My402ListUnlink(&Q2,(void *)curr_packet);
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   curr_pack->Q2_depart=timestr-init_time_d;
   printf("%012.3lfms:p%ld leaves Q2, time in Q2 = %lfms\n",timestr-init_time_d,curr_pack->pac_no,absol(curr_pack->Q2_depart-curr_pack->Q2_entry));
   tot_time_q2+=absol(curr_pack->Q2_depart-curr_pack->Q2_entry);
   
   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   printf("%012.3lfms:p%ld begins service at S2, requesting %lfms of service\n", timestr-init_time_d,curr_pack->pac_no, curr_pack->pac_mu);

   usleep((1/curr_pack->pac_mu)*1000000.00F);
   
   if(die==1){
    pthread_mutex_unlock(&m);
    //printf("s2:CntrlC pressed\n");
    pthread_exit((void *)1);
   }

   gettimeofday(&timer,NULL);
   timestr=timestamp(timer);
   curr_pack->depart_time=timestr-init_time_d;

   printf("%012.3lfms:p%ld departs from S2, service time=%lfms, time in system = %lfms\n", timestr-init_time_d,curr_pack->pac_no, curr_pack->pac_mu,absol(curr_pack->entry_time-curr_pack->depart_time));
   tot_ser_s2+=(curr_pack->pac_mu);
   tot_time_s2+=absol(curr_pack->Q2_depart-curr_pack->depart_time);
   tot_time_sys+=absol(curr_pack->entry_time-curr_pack->depart_time);
   tot_sys_sqr=absol(curr_pack->entry_time-curr_pack->depart_time)*absol(curr_pack->entry_time-curr_pack->depart_time);
   served=curr_pack->pac_no;
   //printf("served:%ld\n", served);
  }

 }
 pthread_mutex_unlock(&m);
 return NULL;
}

int validate_tsfile(FILE *tptr){
 //check tfile specification 
  char *line, *tok_end;
  char *param_tok;
  long int finp; 
  int param_count=0;
  double fval=0.0;
  long int i=0;
  line=(char *)malloc(60*sizeof(char)); //maybe inc the limit

//check if num is +ve
  fgets(line,60,tptr);
  finp=strtol(line, &tok_end, 10); 
  if(tok_end<line+(sizeof(char)*strlen(line))-1){
   printf("input file is not in the right format\n");
   return 1;
  } 
  else if(finp<=0){
   printf("input file is not in the right format\n");
   return 1;
  }
  else 
   num=finp;

  while(i<num){
   fgets(line,60,tptr);
   i++;
   //parsing lambda
   param_tok=strtok(line,"	 ");
    
   fval=strtof(param_tok,&tok_end);
   
   if(tok_end<param_tok+(sizeof(char)*strlen(param_tok))-1){
    printf("lambda:input file is not in the right format\n");
    return 1;
   } 
   else if(fval<=0.0){
    printf("lambda:input file is not in the right format\n");
    return 1;
   }
   else{
    lambda=fval;
    if(lambda<0.1)
     lambda=0.1;
    param_count++;
   }
    //parsing P
   param_tok=strtok(NULL,"	 ");
   finp=strtol(param_tok, &tok_end,10);
   
   if(tok_end<param_tok+(sizeof(char)*strlen(param_tok))-1){
    printf("P:input file is not in the right format\n");
    return 1;
   }
   else if(finp<=0){
    printf("P:input file is not in the right format\n");
    return 1; 
   }
   else{
    P=finp;
    param_count++;
   }
    //parsing mu
   param_tok=strtok(NULL,"	 ");
   fval=strtof(param_tok,&tok_end);
   
   if(tok_end<param_tok+(sizeof(char)*strlen(param_tok))-1){
    printf("mu:input file is not in the right format\n");
    return 1;
   }
   else if(fval<=0.0){
    printf("mu:input file is not in the right format\n");
    return 1;
   }
   else{
    mu=fval;
    if(mu<0.1)
     mu=0.1;
    param_count++;
   }
   
  }
  if(param_count!=3*num){
   printf("input file is not in the right format\n");
   return 1;
  }
 return 0;
}

void * monitor(){
 //int sig;
 while(1){
  //sigwait(&set, &sig);
  sigwait(&set);
  pthread_mutex_lock(&m);
  //critical section
  die=1;
  pthread_mutex_unlock(&m);
 }
 return NULL;
}

void print_statistics(double timestr){
 printf("Statistics:\n");
 printf("\taverage packet inter-arrival time = ");
 printf("%.6g\n", tot_time_lambda/(timestr-init_time_d));
 printf("\taverage packet service time = ");
 printf("%.6g\n",(tot_ser_s1+tot_ser_s2)/(timestr-init_time_d));
 printf("\n");
 printf("\taverage number of packets in Q1 = ");
 printf("%.6g\n",tot_time_q1/(timestr-init_time_d));
 printf("\taverage number of packets in Q2 = ");
 printf("%.6g\n",tot_time_q2/(timestr-init_time_d));
 printf("\taverage number of packets at S1 = ");
 printf("%.6g\n",tot_time_s1/(timestr-init_time_d));
 printf("\taverage number of packets at S2 = ");
 printf("%.6g\n",tot_time_s2/(timestr-init_time_d));
 printf("\n");
 printf("\taverage time a packet spent in system = ");
 printf("%.6g\n",tot_time_sys/(timestr-init_time_d));
 double stnd_dev=tot_sys_sqr/(timestr-init_time_d);
 stnd_dev-=tot_time_sys/(timestr-init_time_d);
 printf("\tstandard deviation for time spent in system = ");
 if(stnd_dev>0){
  printf("%.6g\n",sqrt(stnd_dev));
 }
 else{
  printf("0.000000\n");
 }
 printf("\n");
 printf("\ttoken drop probability = ");
 printf("%.6g\n", (double)drop_tok/tokens);
 printf("\tpacket drop probability = ");
 printf("%.6g\n", (double)drop_pac/packets);
}

int main(int argc, char * argv[]){
 int i=1;
 FILE *tptr;
 struct timeval timer;
 double timestr;
 
 char *line=NULL;
 pthread_t pack_arr, token_arr, s1,s2 ,interrupt_handle;
 sigemptyset(&set);
 sigaddset(&set, SIGINT); 
 
 line=(char *)malloc(60*sizeof(char));
 
 sigprocmask(SIG_BLOCK, &set, 0);
 pthread_create(&interrupt_handle,0,monitor,0);
 
//handle commandline arguments
 while(i<argc){
  if (strcmp(argv[i], "-lambda")==0){
   i++;
   if(i<argc){			
    lambda=atof(argv[i]); //use strtof()
    if(lambda<0.1)
     lambda=0.1;
   }
   else{
    printf("usage:malformed command\n");
    exit(1);
   }
   i++;
  }
  else if(strcmp(argv[i], "-mu")==0){
   i++;
   if(i<argc){			
    mu=atof(argv[i]);
    if(mu<0.1)
     mu=0.1;
   }
   else{
    printf("usage:malformed command\n");
    exit(1);
   }			
   i++;		
  }
  else if(strcmp(argv[i], "-r")==0){
   i++;			
   if(i<argc)			
    r=atof(argv[i]); //use strof()??
   else{
    printf("usage:malformed command\n");
    exit(1);
   }
   i++;
   if(r<0.1)
    r=0.1;
  }
  else if(strcmp(argv[i], "-B")==0){
   i++;
   if(i<argc)			
    B=atoi(argv[i]); //use strtoi()??
   else{
    printf("usage:malformed command\n");
    exit(1);
   }			
   i++;
  }
  else if(strcmp(argv[i], "-P")==0){
   i++;
   if(i<argc)			
    P=atoi(argv[i]);
   else{
    printf("usage:malformed command\n");
    exit(1);
   }			
   i++;
  }
  else if(strcmp(argv[i], "-n")==0){
   i++;
   if(i<argc)			
    num=atoi(argv[i]);
   else{
    printf("usage:malformed command\n");
    exit(1);
   }			
   i++;
  }
  else if(strcmp(argv[i], "-t")==0){
   i++;
   if(i<argc){
    struct stat path_stat;
    stat(argv[i], &path_stat);
    tptr=fopen(argv[i],"r");
    if(tptr==NULL){
     if(errno==ENOENT)
      printf("usage:input file %s does not exist\n", argv[i]);
     else if(errno==EACCES)
      printf("usage: input file %s cannot be opened -access denies\n", argv[i]);
			
     exit(1);
    }
    if(S_ISREG(path_stat.st_mode)==0){
     printf("usage:input file %s is a directory\n", argv[i]);
     exit(1);
    }
   }
   else{
    printf("usage:malformed command\n");
    exit(1);
   }
   filename=argv[i];
   trace_driven=1;
   i++;
  }
  else{
   printf("usage: malformed command\n");
   exit(1);		
  } 
 }

 if(trace_driven==1){
  int wrong=2;
  wrong=validate_tsfile(tptr);
  if(wrong==1){
   fclose(tptr);
   exit(1);
  }
  else if(wrong==0){
   fclose(tptr);
//start thread emulation
   printf("Emulation Parameters:\n");
   printf("\ttsfile = %s\n", argv[argc-1]);
   tptr= fopen(filename,"r");
   num=atoi(fgets(line,60,tptr)); 
  }    
 }
 else{
  printf("Emulation Parameters:\n");
  printf("\tnumber to arrive = %ld\n", num);
  printf("\tlambda = %lf\n", lambda);
  printf("\tmu = %lf\n",mu);
  printf("\tr = %lf\n", r);
  printf("\tB = %ld\n", B);
  printf("\tP = %ld\n", P);
 }
 
 My402ListInit(&Q1);
 My402ListInit(&Q2);
 
 gettimeofday(&init_time,NULL);
 init_time_d=timestamp(init_time);
 printf("00000000.000ms:emulation begins\n");

 pthread_create(&pack_arr,0,pop_q1,0);
 pthread_create(&token_arr,0,pop_buck,0);
 pthread_create(&s1, 0, s1_service, 0);
 pthread_create(&s2, 0, s2_service, 0);
 
 pthread_join(pack_arr,0);	
 pthread_join(token_arr,0);
 
 pthread_mutex_lock(&m);
 if(My402ListEmpty(&Q2)){
  pthread_cancel(s1);
  pthread_cancel(s2);
 }
 pthread_mutex_unlock(&m);

 pthread_mutex_lock(&m);
 if(die==1){
  pthread_mutex_unlock(&m);
  pthread_cancel(s1);
  pthread_cancel(s2);
  
  gettimeofday(&timer,NULL);
  timestr=timestamp(timer);
  print_statistics(timestr);

  My402ListUnlinkAll(&Q1);
  My402ListUnlinkAll(&Q2);
  //printf("Q1:%ld\n", My402ListLength(&Q1));
  exit(1);
 }
 
 /*pthread_join(s1,0);
 pthread_join(s2,0);*/
 
 gettimeofday(&timer,NULL);
 timestr=timestamp(timer);
 printf("%012.3lfms:emulation ends\n",timestr-init_time_d);
 print_statistics(timestr);

 return 0;
}
