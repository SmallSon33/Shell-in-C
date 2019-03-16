/*Youming Zhang
  UID: 114537867
  CMSC 0301
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sysexits.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include "command.h"
#include "executor.h"

static void print_tree(struct tree *t);

int execute(struct tree *t) {
  pid_t pid;
  int status,fd_in=-1,fd_out=-1;

  /*check the tree*/
  if(t==NULL){
    return 0;
  }
  

    
    /********NONE conjunction**********/
    if(t->conjunction==NONE){
      /*exit command*/
      if(strncmp(t->argv[0],"exit",4)==0){
	exit(EXIT_SUCCESS);
      }
      /*cd command*/
      else if(strcmp(t->argv[0],"cd")==0){
	if(t->argv[1]==NULL){/*go home */
	  chdir(getenv("HOME"));
	}else{
	  if((chdir(t->argv[1]))<0){
	    perror("No such directory");
	    exit(EX_OSERR);
	  }
	}
      }else{
	if((pid=fork())<0){
	  perror("fork error");
	  exit(EX_OSERR);
	}
	if(pid){/*parent code*/
	  wait(&status);
	  if (WEXITSTATUS(status)) {
	    fflush(stdout);
	    return -1;
	  }
	  return status;
	}else{
	  /*********check input**********/
	  if(t->input!=NULL){
	    /*open fd*/
	    if((fd_in=open(t->input,O_RDONLY))<0){
	      perror("read fail");
	      exit(EXIT_FAILURE);
	    }
	    /*dup2*/
	    if((dup2(fd_in,STDIN_FILENO))<0){
	      perror("dup2 fail");
	      exit(EXIT_FAILURE);
	    }
	  }/*end input*/
	   close(fd_in);
	  /********check ouput**********/
	  if(t->output!=NULL){
	    /*open fd*/
	    if((fd_out=open(t->output,O_WRONLY|O_CREAT|O_TRUNC,0664))<0){
	      perror("read fail");
	      exit(EXIT_FAILURE);
	    }
	    /*dup2*/
	    if((dup2(fd_out,STDOUT_FILENO))<0){
	      perror("dup2 fail");
	      exit(EXIT_FAILURE);
	    }
	  }/*end output*/
	 
	  close(fd_out);
	  
	  /*system("ps -l");*/
	  if(execvp(t->argv[0],t->argv) < 0){
	    fprintf(stderr,"Fail to execute %s \n",t->argv[0]);
	    fflush(stdout);
	    exit(EXIT_FAILURE);
	  } /*end child*/
	  exit(EXIT_SUCCESS);
	}
      }
    }/*end NONE*/
    
    /**************AND************/
    if(t->conjunction==AND){
      /*********check input**********/
      if(t->input!=NULL){
	/*open fd*/
	if((fd_in=open(t->input,O_RDONLY))<0){
	  perror("read fail");
	  exit(EXIT_FAILURE);
	}
	/*dup2*/
	if((dup2(fd_in,STDIN_FILENO))<0){
	  perror("dup2 fail");
	  exit(EXIT_FAILURE);
	}
      }/*end input*/
      close(fd_in);
      /********check ouput**********/
      if(t->output!=NULL){
	/*open fd*/
	if((fd_out=open(t->output,O_WRONLY|O_CREAT|O_TRUNC,0664))<0){
	  perror("read fail");
	  exit(EXIT_FAILURE);
	}
	/*dup2*/
	if((dup2(fd_out,STDOUT_FILENO))<0){
	  perror("dup2 fail");
	  exit(EXIT_FAILURE);
	}
      }/*end output*/
      close(fd_out);
      if(execute(t->left)==0){
	execute(t->right);
      }
      return 0;
    }/*end AND conjunction*/
    
    /************OR***************/
    if(t->conjunction==OR){
     
    }
    /***********SEMI*************/
    if(t->conjunction==SEMI){
      
    }
    /***********PIPE************/
    if(t->conjunction==PIPE){
      int fd[2];
      pid_t tmp_pid;

        /*********check input**********/
	  if(t->input!=NULL){
	    /*open fd*/
	    if((fd_in=open(t->input,O_RDONLY))<0){
	      perror("read fail");
	      exit(EXIT_FAILURE);
	    }
	    /*dup2*/
	    if((dup2(fd_in,STDIN_FILENO))<0){
	      perror("dup2 fail");
	      exit(EXIT_FAILURE);
	    }
	  }/*end input*/
	   close(fd_in);
	  /********check ouput**********/
	  if(t->output!=NULL){
	    /*open fd*/
	    if((fd_out=open(t->output,O_WRONLY|O_CREAT|O_TRUNC,0664))<0){
	      perror("read fail");
	      exit(EXIT_FAILURE);
	    }
	    /*dup2*/
	    if((dup2(fd_out,STDOUT_FILENO))<0){
	      perror("dup2 fail");
	      exit(EXIT_FAILURE);
	    }
	  }/*end output*/
	 
	  close(fd_out);
	  
      if(t->left->output!=NULL){
	printf("Ambiguous output redirect.\n");
	return EXIT_FAILURE;
      }
      if(t->right->input!=NULL){
	printf("Ambiguous intput redirect.\n");
	return EXIT_FAILURE;
      }
      /*piping*/
      if(pipe(fd)<0){
	perror("pipe error");
      }
      /*fork*/
      if((tmp_pid=fork())<0){
	perror("fork error");
      }

      if(tmp_pid){/*parent code*/
	wait(&tmp_pid);
	close(fd[1]); /*close write*/
	dup2(fd[0],STDIN_FILENO);
	execute(t->right);
	close(fd[0]);

      }else{
	close(fd[0]);
	dup2(fd[1],STDOUT_FILENO);
	execute(t->left);
	close(fd[1]);

      }
    
      return 0;
    }/*end piping*/
    /******************SUBSHELL**********************/
    if(t->conjunction==SUBSHELL){
      int pid2,status2;
      /*********check input**********/
      if(t->input!=NULL){
	/*open fd*/
	if((fd_in=open(t->input,O_RDONLY))<0){
	  perror("read fail");
	  exit(EXIT_FAILURE);
	}
	/*dup2*/
	if((dup2(fd_in,STDIN_FILENO))<0){
	  perror("dup2 fail");
	  exit(EXIT_FAILURE);
	}
      }/*end input*/
      close(fd_in);
      /********check ouput**********/
      if(t->output!=NULL){
	/*open fd*/
	if((fd_out=open(t->output,O_WRONLY|O_CREAT|O_TRUNC,0664))<0){
	  perror("read fail");
	  exit(EXIT_FAILURE);
	}
	/*dup2*/
	if((dup2(fd_out,STDOUT_FILENO))<0){
	  perror("dup2 fail");
	  exit(EXIT_FAILURE);
	}
	
      }/*end output*/
	 
      close(fd_out);
      
      if((pid2=fork())<0){
	perror("fork error");
	exit(EX_OSERR);
      }
      if(pid2){
	wait(&pid2);
      }else{
	execute(t->left);
        exit(EXIT_SUCCESS);
      }
      return 0;
    }/*end subshell*/
    
 
   return 0;
}

static void print_tree(struct tree *t) {
   if (t != NULL) {
      print_tree(t->left);

      if (t->conjunction == NONE) {
         printf("NONE: %s, ", t->argv[0]);
      } else {
         printf("%s, ", conj[t->conjunction]);
      }
      printf("IR: %s, ", t->input);
      printf("OR: %s\n", t->output);

      print_tree(t->right);
   }
}

