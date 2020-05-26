#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }

#define LIMIT 100000
#define RESET_LIMIT 100

sem_t *process_semaphore;
int counter;
int reset;

void* thread_function(void *arg) {

	while (1) {

		if (reset > RESET_LIMIT-1) {
			printf("limite raggiunto\n");
			break;
		}

		if (sem_wait(process_semaphore) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}

		if (counter > LIMIT) {
			counter = counter - LIMIT;
			reset++;
		}

		counter++;

		if (sem_post(process_semaphore) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}

	}

	return NULL;
}

int main(int argc, char *argv[]) {

	pthread_t t1;
	pthread_t t2;
	void *res;

	int s;

	printf("initial value of counter=%d, reset=%d\n", counter, reset);

	process_semaphore = malloc(sizeof(sem_t));

	s = sem_init(process_semaphore, 0, // 1 => il semaforo è condiviso tra processi,
									   // 0 => il semaforo è condiviso tra threads del processo
			1 // valore iniziale del semaforo (se mettiamo 0 che succede?)
			);

	CHECK_ERR(s, "sem_init")

	s = pthread_create(&t1, NULL, thread_function, NULL);

	if (s != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	s = pthread_create(&t2, NULL, thread_function, NULL);

	if (s != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	s = pthread_join(t1, &res);

	if (s != 0) {
		perror("pthread_join");
		exit(EXIT_FAILURE);
	}

	s = pthread_join(t2, &res);

	if (s != 0) {
		perror("pthread_join");
		exit(EXIT_FAILURE);
	}

	printf("final value of counter=%d\n & value of reset: %d\n", counter,
			reset);

	// il semaforo senza nome va distrutto solo quando non ci sono processi bloccati su di esso
	s = sem_destroy(process_semaphore);
	CHECK_ERR(s, "sem_destroy")

	printf("bye\n");

}

