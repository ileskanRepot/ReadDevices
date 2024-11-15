// https://stackoverflow.com/questions/15949163/read-from-dev-input
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>
#include <sys/time.h>

struct inputEvent {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value;
};

int main(int argc, char *argv[])
{
	int timeoutMs = 5000;
	char inputDev[] = "/dev/input/event0";
	int st;
	int ret;
	struct pollfd fds[1];

	fds[0].fd = open(inputDev, O_RDONLY|O_NONBLOCK);

	if(fds[0].fd<0)
	{
		printf("error unable open for reading '%s'\n",inputDev);
		return(0);
	}

	const int inputSize = sizeof(struct inputEvent);
	// unsigned char inputData[inputSize];
	struct inputEvent inputData[1];
	memset(inputData,0,inputSize);

	fds[0].events = POLLIN;

	int exitOnKeyPressCount = 10;

	while(true) {
		ret = poll(fds, 1, timeoutMs);
		if (ret <= 0){
			printf("timeout\n");
			continue;
		}
		if (!fds[0].revents) {
			printf("error\n");
			continue;
		}

		ssize_t r = read(fds[0].fd,inputData,inputSize);

		if(r<0)
		{
			printf("error %d\n",(int)r);
			break;
		}
		printf("total bytes read %d/%d\n",(int)r,inputSize);

		printf("time=%ld.%06lu type=%hu code=%hu value=%u\n", inputData->time.tv_sec, inputData->time.tv_usec, inputData->type, inputData->code, inputData->value);
		// for(int i = 0; i<r;i++)
		// {
			// printf("%02X ",(unsigned char)inputData[i]);
		// }
		// printf("\n");
		memset(inputData,0,inputSize);

		exitOnKeyPressCount--;
		// if(exitOnKeyPressCount<1)
			// break;
	}

	close(fds[0].fd);
	return 0;
}
