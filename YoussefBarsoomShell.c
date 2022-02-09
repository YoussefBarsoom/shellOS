
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include <dirent.h>
#include <errno.h>
#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
#define MAXREPLAYSTACK 1000 //max number of recursive replay call cmds
//ASSIGNMENT 2 OPERATING SYSTEMS
//Name:Youssef Barsoom
//To run Shell: Execute the following
//[1] sudo apt-get install libreadline-dev
//[2] gcc shell.c -lreadline
//[3]./a.out 
// A linked list node
struct Node {
	char* data;
	struct Node* next;
};

 char currentdir[];
//currentReplayStack is used to avoid infinite replay cmds
//Ex: history.txt content:[0] ...  [1] replay 1
//and I type replay 1 in the shell it will be stuck in an infinite loop 
int currentReplayStack;//keep track of number of recursive replay cmd 
// Greeting shell during startup
void init_shell()
{
    //clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****THE BOLT SHELL****");
    printf("\n\n\n\n*******************"
        "***********************");
 printf("\n\n\n\n Written by Youssef Barsoom");
  
    printf("\n");
    
}


//checking if Dir exsists and changing currentDir
void moveToDir(char* dir)
{

DIR* direc = opendir(dir);
if (direc) {
    /* Directory exists. */
strcpy(currentdir,dir);
printf("%s is the new Directory", currentdir);
} else if (ENOENT == errno) {
    /* Directory does not exist. */
printf("%s Doesn't Exist\n",dir);
} else {
    /* opendir() failed for some other reason. */
printf("Error Occured");
}
}

void printCurrentDir()
{
printf("\n%s is the current directory", currentdir);
}
  
// Function to take input
int takeInput(char* str)
{
char* buf;

buf = readline("\n# ");

    if (strlen(buf) != 0) {

        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}
  

// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork(); 
  
    if (pid == -1) {
        printf("\nFailed forking child..");
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL); 
        return;
    }
}

void execArgsFile(int flag,char** parsed)
{
//Editing filename if getting program from current directory
//Example: start sayMyName Heisenberg 
// location is currentDir/sayMyName
if(parsed[0][0]!='/')
{
char location[PATH_MAX];
strcpy(location,currentdir);
strcat(location,"/");	
strcat(location,parsed[0]);
parsed[0]= location;

}

printf("File location:%s",parsed[0]);	
//if flag=1 then wait Orginal cmd: start
//if flag=0 then return pid immediately Original cmd:background
    pid_t process;
	
    process = fork();
    
    if (process < 0)
    {
        // fork() failed.
        perror("fork");
        return 2;
    }

    if (process == 0)
    {
        // sub-process

	printf("%s",parsed[0]);

        execvp(parsed[0], parsed);
        perror("execvp"); // Ne need to check execv() return value. If it returns, you know it failed.
        return 2;
    }

    int status;
    pid_t wait_result;

if(flag)
{
        printf("Waiting for process to terminate...\n");
  while ((wait_result = waitpid(process,&status,WNOHANG)) != -1)
    {

    }
    printf("Process Terminated\n");  
}
  
    printf("Process ID:%d\n",(int) process);


    return 0;
}
  



void showHistory(char* parsed)
{
FILE * fp;

if(parsed)
{
//Clear History Condition
if(strcmp(parsed, "[-c]") == 0)
{
printf("Clearing History...\n");
char location[PATH_MAX];
strcpy(location,currentdir);
strcat(location,"/history.txt");
fp = fopen("history.txt", "w");

    //clears file
    fprintf(fp,"");
return;
}
else
{
printf("Invalid History Command\n");
return;
}
}

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    struct Node *head=NULL;
char location [PATH_MAX];
strcpy(location,currentdir);
strcat(location,"/history.txt");
fp = fopen("history.txt", "r");

    if (fp == NULL)
{
	printf("FAILLLLL");
        exit(EXIT_FAILURE);
}

   while ((read = getline(&line, &len, fp)) != -1) {

//char * c = line;
   addFirst(&head,line);


}

 
struct Node *temp= head;
int i=0;
   while(temp != NULL)
    {
	//char c[100] = temp->data;
         printf("[%d] %s", i,((temp)->data));
         temp = temp->next;
	i++;
    }

    fclose(fp);
}


void killProcess(char* pidToKillPtr)
{
int pidToKill= atoi(pidToKillPtr);
//kill returns 0 if sucessfull
  int status;
    pid_t wait_result;
if(pidToKill==NULL)
{
printf("No PID is entered\n",pidToKill);
return;
}
if(!(kill(pidToKill,SIGKILL)))
{  while ((wait_result = waitpid(pidToKill,&status,WNOHANG)) != -1)
    {

    }
printf("Process %d Successfully Terminated\n",pidToKill);}
else{
printf("Process %d could not be Terminated\n",pidToKill);}
return;
}



void replay(char * cmdNumString)
{
//we are adding replay to history before even executing replay thus the index will be shifted by 1
//The issue with replay is that it could end up in an infinite loop
//Ex:   [0]replay 1
//	[1]replay 2 
//	[2]replay 0
// or Command is replay 0 and in history:[0] replay 0;
//these will cause infinte loops, thus to avoid them a counter will be added for the number of  
//recursive replay that can be made. Since not all of them will cause an infinte loop.
// the counter will reset if an execution is made other then replay.
if(currentReplayStack== MAXREPLAYSTACK)
{
printf("Replay Command Stack reached max. Replay probably stuck in an infinite loop.");
return 0;
}
currentReplayStack++;
int cmdNum = atoi(cmdNumString) + 1;
FILE * fp;
char * line = NULL;
size_t len = 0;

ssize_t read;
struct Node *head=NULL;

fp = fopen("history.txt", "r");

    if (fp == NULL)
{
	printf("Folder Failed To Open");
        exit(EXIT_FAILURE);
}
int i =cmdNum;

   while ((read = getline(&line, &len, fp)) != -1) {
	   addFirst(&head,line);
	}


struct Node *temp= head;

   while((temp->next) != NULL &&i!=0)
    {
	
         temp = temp->next;
	i--;
    }

if(i>0)
{

printf("Number: %d does not exist in history\n",(cmdNum-1));
}
else if(i==0)
{
//if()
strcspn(temp->data," ");

//printf(":::::%s  %d:;;;;;;;",line,len);
char *parsedArgs[MAXLIST];
printf("Replaying: %s \n",temp->data);
int execFlag = processString(temp->data,parsedArgs);




}
return;
}

void addFirst(struct node **head, char* val)
{
      //create a new node
      struct Node *newNode = malloc(sizeof(struct Node));
     // newNode->data = val;
 newNode->data = strdup(val);

      newNode->next = *head;
      
      *head = newNode;
      

}

void addCmdToHistory(char cmd[])
{
FILE * fp;

fp = fopen("history.txt", "a");
fprintf(fp,"%s\n",cmd);
fclose(fp);
}
int validateNumOfInputs(int numOfParams,int numOfParamsNeeded)
{
//function will be used for all commands except for:
//start,background
if(numOfParams==numOfParamsNeeded){
return 1;
}

printf("Incompatible Number of parameters");
return 0;
}


// Function to execute builtin commands
int ownCmdHandler(char** parsed,int numOfParams)
{
    int NoOfOwnCmds = 8, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];

    char* username;


//ARGUMENT NULL IS NEEDED AS THE LAST ARGUMENT FOR EXECVP() for execArgsFile function
    ListOfOwnCmds[0] = "byebye";
    ListOfOwnCmds[1] = "movetodir";
    ListOfOwnCmds[2] = "whereami";
    ListOfOwnCmds[3] = "history";
    ListOfOwnCmds[4] = "replay";
    ListOfOwnCmds[5] = "start";
    ListOfOwnCmds[6] = "background";
    ListOfOwnCmds[7] = "dalek";
    for (i = 0; i < NoOfOwnCmds; i++) {

        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }


    switch (switchOwnArg) {
    case 1:

if(validateNumOfInputs(numOfParams,0))
{

        printf("\nGoodbye\n");
        exit(0);
}

    case 2:
if(validateNumOfInputs(numOfParams,1))
{

        moveToDir(parsed[1]);
}
        return 1;
    case 3:
if(validateNumOfInputs(numOfParams,0))
{

	printCurrentDir();
}
        return 1;
    case 4:
if(numOfParams<=1)
{
//checking with 1 if history [-c] and checking with 0 if just history
// if either return true we execute
	showHistory(parsed[1]);
}
else
{
printf("Incompatible Number of parameters\n");
}
        return 1;
    case 5:
if(validateNumOfInputs(numOfParams,1))
{

	replay(parsed[1]);
}
        return 1;
    case 6:

	execArgsFile(1,&parsed[1]);
        return 1;
    case 7:
	execArgsFile(0,&parsed[1]);
        return 1;
    case 8:
if(validateNumOfInputs(numOfParams,1))
{

	killProcess(parsed[1]);
}
        return 1;
    default:
        break;
    }
  
    return 0;
}
  

// function for parsing command words and returns number of paramaters 
int parseSpace(char* str, char** parsed)
{
    int i;
int num=0;
//when replaying from history, history contains an \n after each command to distinguish each line
//however it is also read when reading history file thus removing it 
//it will not be confused with any '/' used for file location 
    str= strsep(&str, "\n");
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
	{

            break;

	}
        if (strlen(parsed[i]) == 0)
            {i--;
	num--;}
num++;
    }
//returns number of parameters excluding the cmd itself
// replay 1 2 3 num = 3 not 4 excluding replay
return num-1;
}
  
int processString(char* str, char** parsed)
{
     
   


    

    if (ownCmdHandler(parsed,parseSpace(str, parsed)))

        return 0;
	
    else
        return 1;
}
  
int main()
{
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    char cwd[PATH_MAX];
    init_shell();
  char tempString[]="";
    getcwd(cwd, sizeof(cwd));
    printf("CWD: %s",cwd);
    strcpy(currentdir,cwd);
    while (1) {
        // take input
        if (takeInput(inputString))	
	
            continue;
        // process
	//strcat(tempString,inputString);	
	addCmdToHistory(inputString);
	currentReplayStack =0;
        execFlag = processString(inputString,parsedArgs);
        // execute
        if (execFlag == 1)
            {execArgs(parsedArgs);}
	
  
        
    }
    return 0;
}