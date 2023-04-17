#include <can_demo.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	int pid;
	char user_input[15];

	printf("CAN Sockets Demo\r\n");
	printf("Send string \"hello\" via CAN : \r\n");
    strcpy(user_input, "hello");

	pid = fork();
	if (pid > 0) {
        /*parent process*/
        receive();
    }
    else if (pid == 0) {
        /*child process*/
		sleep(3);
		transmit(user_input);
    }
    else {
        /*fork creation failed*/
        printf("the fork that is failed!!\n");
    }

	return 0;
}
