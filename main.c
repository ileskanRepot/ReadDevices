// https://stackoverflow.com/questions/15949163/read-from-dev-input
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>

struct inputEvent {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value;
};

enum charKeyCodes {rightCode = 32, leftCode = 30, upCode = 17, downCode = 31, exitCode = 16};
enum listIndex {rightIndex = 0, leftIndex = 1, upIndex = 2, downIndex = 3, exitIndex = 4};

void takeInput(char* shm)
{
	int timeoutMs = 5000;
	char inputDev[] = "/dev/input/event0";
	int st;
	int ret;
	struct pollfd fds[1];


	shm[rightIndex] = 0;
	shm[leftIndex] = 0;

	fds[0].fd = open(inputDev, O_RDONLY|O_NONBLOCK);

	if(fds[0].fd<0)
	{
		printf("error unable open for reading '%s'\n",inputDev);
		return;
	}

	const int inputSize = sizeof(struct inputEvent);
	// unsigned char inputData[inputSize];
	struct inputEvent inputData[1];
	memset(inputData,0,inputSize);

	fds[0].events = POLLIN;

	int exitOnKeyPressCount = 10;

	while(1) {
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
		// printf("total bytes read %d/%d\n",(int)r,inputSize);
		if (
				(inputData->type == 0 && inputData->code == 0 && inputData->value == 0) 
				|| (inputData->value == 2)
				|| (inputData->type != 1)
				// || (inputData->value == 2)
				) {
			continue;
		}
		// printf("type=%hu code=%hu value=%u\n", inputData->type, inputData->code, inputData->value);

		switch (inputData->code) {
			case rightCode:
				shm[rightIndex] = inputData->value;
				break;
			case leftCode:
				shm[leftIndex] = inputData->value;
				break;
			case upCode:
				shm[upIndex] = inputData->value;
				break;
			case downCode:
				shm[downIndex] = inputData->value;
				break;
			default:
				break;
		}

		memset(inputData,0,inputSize);

		exitOnKeyPressCount--;
		// if(exitOnKeyPressCount<1)
			// break;
	}

	close(fds[0].fd);
	return;
}

void printVec(char*vec, int size){
	for (int ii = 0; ii<size; ii++){
		printf("%d ", vec[ii]);
	}
	printf("\n");
}

void physics(char* shm) {
	const int max = 10;

	int playerPosX = 0;
	int playerPosY = 0;
	while (1){
		// printf("Left %d Right %d\n", shm[leftIndex], shm[rightIndex]);
		// printVec(shm, SHMSIZE);

		if (shm[leftIndex]){
			if (playerPosX != 0)
				playerPosX -= 1;
		}
		if (shm[rightIndex]){
			if (playerPosX != max-1)
				playerPosX += 1;
		}
		if (shm[upIndex]){
			if (playerPosY != 0)
				playerPosY -= 1;
		}
		if (shm[downIndex]){
			if (playerPosY != max-1)
				playerPosY += 1;
		}

		for (int yy = 0; yy < max;yy++){
			for (int xx = 0; xx < max;xx++){
				if (xx == playerPosX && yy == playerPosY){
					printf("P");
				}else{
					printf("N");
				}
			}
			printf("\n");
		}
		printf("\n");
		usleep(0.1 * 1000 * 1000);
	}
}

int main(int argc, char *argv[]) {
	char* shmem = (char*)mmap(
			NULL, 
			8, 
			PROT_READ | PROT_WRITE, 
			MAP_SHARED | MAP_ANON, 
			-1, 
			0
	);

	pid_t pp = fork();
	if (pp < 0){
		perror("Fork failed");
		exit(1);
	}


	if (pp == 0){
		// usleep(0.1 * 1000 * 1000);
		// printf("%p %d\n", shmem, *(char*)shmem);
		physics(shmem);
	}else{
		// printf("%p %d\n", shmem, *(char*)shmem);
		takeInput(shmem);
		// usleep(0.1 * 1000 * 1000);
	}
}
