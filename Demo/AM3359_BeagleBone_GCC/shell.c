#include "shell.h"
#include <stddef.h>
// #include "clib.h"
// #include <string.h>
// #include "fio.h"
// #include "filesystem.h"
#include "serial.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "mmu_dump.c"
// #include "host.h"

#include <math.h>
xTaskHandle blinkhandle = NULL;
typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;
void mmudump_command(int, char **);
void blink_command(int, char**);
void addr_command(int, char**);
void mmuinit_command(int , char **);
// void ls_command(int, char **);
// void man_command(int, char **);
// void cat_command(int, char **);
// void ps_command(int, char **);
// void host_command(int, char **);
void help_command(int, char **);
// void host_command(int, char **);
// void mmtest_command(int, char **);
void test_command(int, char **);
void _command(int, char **);

// void new_command(int, char **);
// void do_sth(void *);
// void ps_record(void *);

int stoi(char *); //make sting of number into integer form
float InvSqrt(int);	//compute the sqrt of a number.

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
    MKCL(mmudump, "dump the mmu!"),
    MKCL(blink, "led blinking"),
    MKCL(addr, "print address..."),
    MKCL(mmuinit, "initial the mmu"),
	// MKCL(ls, "List directory"),
	// MKCL(man, "Show the manual of the command"),
	// MKCL(cat, "Concatenate files and print on the stdout"),
	// MKCL(ps, "Report a snapshot of the current processes"),
	// MKCL(host, "Run command on host"),
	// MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "test new function"),
	// MKCL(new, "Create new task"),
	MKCL(, ""),
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}
void blink(void *pvParameters)
{
    const portTickType msDelay = 500;
    unsigned int i;
    serial_puts(UART0_BASE,"blinktask\n");
    while(1)
    {

            (*(REG32(GPIO1_BASE + GPIO_CLEARDATAOUT))) |= PIN21;
            i=0x1FFFF;
            while(i--){;}
            (*(REG32(GPIO1_BASE + GPIO_SETDATAOUT))) |= PIN21;
            i=0x1FFFF;
            while(i--){;}
    }
}
// void ls_command(int n, char *argv[]){
//     io_printf(UART0_BASE,"\r\n");
//     int dir;
//     if(n == 1){
//         dir = fs_opendir("");
//     }else if(n == 2){
//         dir = fs_opendir(argv[1]);
//         //if nothing print out, then do not print new line.
//         if(dir > 0) io_printf(UART0_BASE,"\r\n");
//     }else{
//         io_printf(UART0_BASE, "Too many argument!\r\n");
//         return;
//     }
// (void)dir;   // Use dir
// }

// int filedump(const char *filename){
// 	char buf[128];

// 	int fd=fs_open(filename, 0, O_RDONLY);

// 	if( fd == -2 || fd == -1)
// 		return fd;

// 	io_printf(UART0_BASE, "\r\n");

// 	int count;
// 	while((count=fio_read(fd, buf, sizeof(buf)))>0){
// 		fio_write(1, buf, count);
//     }

//     io_printf(UART0_BASE, "\r");

// 	fio_close(fd);
// 	return 1;
// }

// void ps_command(int n, char *argv[]){
// 	signed char buf[1024];
// 	vTaskList(buf);
//         io_printf(UART0_BASE, "\n\rName          State   Priority  Stack  Num\n\r");
//         io_printf(UART0_BASE, "*******************************************\n\r");
// 	io_printf(UART0_BASE, "%s\r\n", buf + 2);
// }

// void cat_command(int n, char *argv[]){
// 	if(n==1){
// 		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
// 		return;
// 	}

//     int dump_status = filedump(argv[1]);
// 	if(dump_status == -1){
// 		fio_printf(2, "\r\n%s : no such file or directory.\r\n", argv[1]);
//     }else if(dump_status == -2){
// 		fio_printf(2, "\r\nFile system not registered.\r\n", argv[1]);
//     }
// }

// void man_command(int n, char *argv[]){
// 	if(n==1){
// 		fio_printf(2, "\r\nUsage: man <command>\r\n");
// 		return;
// 	}

// 	char buf[128]="/romfs/manual/";
// 	strcat(buf, argv[1]);

//     int dump_status = filedump(buf);
// 	if(dump_status < 0)
// 		fio_printf(2, "\r\nManual not available.\r\n");
// }

// void host_command(int n, char *argv[]){
//     int i, len = 0, rnt;
//     char command[128] = {0};

//     if(n>1){
//         for(i = 1; i < n; i++) {
//             memcpy(&command[len], argv[i], strlen(argv[i]));
//             len += (strlen(argv[i]) + 1);
//             command[len - 1] = ' ';
//         }
//         command[len - 1] = '\0';
//         rnt=host_action(SYS_SYSTEM, command);
//         io_printf(UART0_BASE, "\r\nfinish with exit code %d.\r\n", rnt);
//     }
//     else {
//         fio_printf(2, "\r\nUsage: host 'command'\r\n");
//     }
// }

void help_command(int n,char *argv[]){
	int i;
	io_printf(UART0_BASE, "\r\n");
	for(i = 0;i < sizeof(cl)/sizeof(cl[0]) - 1; ++i){
		io_printf(UART0_BASE, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}
void mmuinit_command(int n,char *argv[]){
    start_mmu();
}
void mmudump_command(int n,char *argv[]){
    mmu_dump();
}
void addr_command(int n,char *argv[]){
    int *add1=0xB0000000;
    //int *add2=0xD0000004;
    int tmp=0;
    for(;1;add1+=1){
        *add1=1;
        tmp++;
        if(tmp==0x1000){
        tmp=0;
        io_printf(UART0_BASE,"add1=%p\r\n",add1);
        // for(tmp=0;tmp<1000000;tmp++)
        // {}
         }
    }


    // *add1=12345;
    // io_printf(UART0_BASE,"\n\radd1=12345 ,add2=%d",*add2);
    // *add1=77777;
    // io_printf(UART0_BASE,"\n\radd1=77777 ,add2=%d\r\n",*add2);

}
void blink_command(int n,char *argv[]){
    if(blinkhandle==NULL)
        xTaskCreate(blink,  ( signed char * ) "BLINKing", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &blinkhandle);
    else if(blinkhandle){
        vTaskDelete(blinkhandle);
        blinkhandle=NULL;
    }
}
void test_command(int n, char *argv[]) {

    if (n>=3)
    {
    	/*
	    	compute Fibonacci number with char 'f' and int parameter
	    	e.g. "test f 10" is to compute the 10th number of Fibonacci number.
    	*/
    	if (*argv[1]=='f')
    	{
    		//fit the format and then operate the funciton
    		if (*argv[2]!=0)
			{
				int i;
	    		int count = stoi(argv[2]);
	    		int result = 1;
	    		int prev = 0;
	    		int temp;
	    		for (i = 1; i < count; ++i)
	    		{
	    			temp = result;
	    			result = result + prev;
	    			prev = temp ;
	    		}
	    		//print the result to the screen.
	    		io_printf(UART0_BASE,"\r\nFibonacci number at %d is %d",count,result) ;
			}
    	}

    	/*
    		judge whether the input number is a prime with char 'p' and int parameter.
    		e.g. after type "test p 3", the result will be shown up in the terminal.
    	*/
    	if (*argv[1]=='p')
    	{
    		if (*argv[2]!=0)
    		{
    			int is_prime = 1; //1->is a prime ; 0-> not a prime
    			int number = stoi(argv[2]);

    			if ((number << 31)==0) //check whether it is even number
    			{
    				is_prime = 0;
    				if (number==2)	// 2 is a prime
    				{
    					is_prime = 1;
    				}
    			}
    			else
    			{
    				int i;
    				int count = (int)InvSqrt(number);	// from 2 to sqrt(n)
    				float result;
    				for (i = 2; i <= count; ++i)
    				{
    					result = number%i; //if the remainder is 0, it is not prime
    					if (result==0)
    					{
    						is_prime = 0;
    						break;			//if it is not a prime, the check is over.
    					}
    				}
    			}

    			//print the result to the screen
    			if (is_prime)
    			{
    				io_printf(UART0_BASE,"\r\nNumber %d is a prime.",number);
    			}
    			else
    			{
    				io_printf(UART0_BASE,"\r\nNumber %d is not a prime.",number);
    			}
    		}
    	}

    }

/*
    int handle;
    int error;

    io_printf(UART0_BASE, "\r\n");

    handle = host_action(SYS_SYSTEM, "mkdir -p output");
    handle = host_action(SYS_SYSTEM, "touch output/syslog");

    handle = host_action(SYS_OPEN, "output/syslog", 8);
    if(handle == -1) {
        io_printf(UART0_BASE, "Open file error!\n\r");
        return;
    }

    char *buffer = "Test host_write function which can write data to output/syslog\n";
    error = host_action(SYS_WRITE, handle, (void *)buffer, strlen(buffer));
    if(error != 0) {
        io_printf(UART0_BASE, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
        host_action(SYS_CLOSE, handle);
        return;
    }

    host_action(SYS_CLOSE, handle);*/
}

void _command(int n, char *argv[]){
    (void)n; (void)argv;
    io_printf(UART0_BASE, "\r\n");
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;
}

int stoi(char *str)
{
	int i;
	int result = 0;
	int count = strlen(str);
	for (i = 0; i < count; ++i)
	{
		result = result*10+(str[i]-'0');
	}
	return result;
}

float InvSqrt(int x_in)
{
	float x = (float)x_in;
    float xhalf = 0.5f*x;
    int i = *(int*)&x; // get bits for floating VALUE
    i = 0x5f375a86- (i>>1); // gives initial guess y0
    x = *(float*)&i; // convert bits BACK to float
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    return 1/x;
}

// void new_command(int n, char *argv[])
// {
// 	io_printf(UART0_BASE,"\r\n");
// 	if (n>=2)
// 	{
// 		int count = stoi(argv[1]);
// 		int i;
// 		int error_code; //detect whether the task created by the return code
// 		for (i = 0; i < count; ++i)
// 		{
// 			error_code = xTaskCreate(do_sth,	//task ptr
// 									(signed portCHAR *)"do_sth",	//task name
// 									512,	//stack depth
// 				 					NULL,	//pass the parameter to the funciton
// 				 					tskIDLE_PRIORITY + 1,	//priority of the task
// 				 					NULL);
// 			if (error_code == 1)
// 			{
// 				io_printf(UART0_BASE,"A new task has been created. \r\n");
// 			}
// 			else
// 			{
// 				io_printf(UART0_BASE,"A new task fail to be created.\r\n");
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if (xTaskCreate(ps_record,	(signed portCHAR *)"ps_rec",	512, NULL, tskIDLE_PRIORITY + 1, NULL) == 1)
// 		{
// 			io_printf(UART0_BASE,"Process-State Rocord Task created.\r\n");
// 		}
// 	}
// }

// void do_sth(void *pvParameters)
// {
// 	while(1)
// 	{
// 		//do nothing.
// 	}
// }

// void ps_record(void *pvParameters)
// {
// 	while(1)
// 	{
// 		signed char buf[1024];
// 		vTaskList(buf);

// 		int buf_len = 0;
// 		while(*(buf+buf_len)!='\0')
// 		{
// 			buf_len ++;	//to find the end mark of the char matrix.
// 		}

// 		int handle;
// 	    int error;

// 	    handle = host_action(SYS_SYSTEM, "mkdir -p output");
// 	    handle = host_action(SYS_SYSTEM, "touch output/sysinfo");

// 	    handle = host_action(SYS_OPEN, "output/sysinfo", 8);
// 	    if(handle == -1) {
// 	        io_printf(UART0_BASE, "Open file error!\n\r");
// 	        return;
// 	    }

// 	    char *buffer1 = "\nName          State   Priority  Stack  Num\n";
// 	    error = host_action(SYS_WRITE, handle, (void *)buffer1, strlen(buffer1));
// 	    if(error != 0) {
// 	        io_printf(UART0_BASE, "Write file error! Remain %d bytes didn't write in the file.\n\r", error);
// 	        host_action(SYS_CLOSE, handle);
// 	        return;
// 	    }
// 	    char *buffer2 = "*******************************************";
// 	    host_action(SYS_WRITE, handle, (void *)buffer2, strlen(buffer2));
// 	    host_action(SYS_WRITE, handle, (void *)buf, buf_len);

// 	    host_action(SYS_CLOSE, handle);

// 	    /*
// 	    	delay 1 sec.
// 	    	xTaskGetTickCount() will add 1 every 10ms,
// 	    	so it takes 1 sec after add 100.
// 	    */
// 		while(xTaskGetTickCount()%100);
// 	}
// }