#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>

#include "cards.h"

#define PORT_NO 5000
#define MAX_PLAYERS 400
#define QLEN 5

unsigned int activeconn = 0;

int main(int argc, char *argv[])
{
    int msock, newsock;
    int i;
    int player_sockets[MAX_PLAYERS];
    struct sockaddr_in serv_addr;
    struct timeval timeout;

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    
    for(i = 0; i < MAX_PLAYERS; i++)
	    player_sockets[i] = 0; 

    msock = socket(AF_INET, SOCK_STREAM, 0);
    if(msock < 0)
	    errexit("Error creating socket\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NO);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(msock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	    errexit("Bind error\n");

    listen(msock, QLEN);
    
    while(1)
    {
	newsock = accept(msock, (struct sockaddr*)NULL, NULL);

	if (newsock < 0) {
		printf("accept failed\n");
		sleep(1);
		continue;
	}

	setsockopt(newsock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
	activeconn++;
	for(i = 0; i < MAX_PLAYERS; i++)   {
		if(player_sockets[i] == 0)	{
			player_sockets[i] = newsock;
			break;
		}
	}
	pthread_t t;
	pthread_create(&t, NULL, (void *) &playgame, (void *)&newsock);
    }
}

void shuffle(deck_t *deck)	{

	int i, j, temp;
	time_t t;

	for(i = 0; i < DECKSIZE;i++)
		deck->cards[i] = i;

	// Shuffle unshuffled deck using Fisher-Yates Shuffle

	srand ((unsigned) time (&t));

	// shuffle deck; iterate downward through deck, move card at location i to random location less than i.
	for(i = DECKSIZE - 1; i > 0; i--)	{
		j = rand() % i;

		temp = deck->cards[j];
		deck->cards[j] = deck->cards[i];
		deck->cards[i] = temp;
	}
}

#define SIZE_HIT	((MAX_PLAYERS_PER_GAME + 1) * sizeof(int))

int start(int fd, hand_t *hands, deck_t *deck, int *hit)
{
	int i, res = -1;

	for(i = 0; i < MAX_PLAYERS_PER_GAME; i++)
	{
		hit[i] = 1;
		memset(&hands[i], 0, sizeof(hand_t));
	}
	hit[MAX_PLAYERS_PER_GAME] = 0; // When this is 1, that is the message to the client that the game is over.

	shuffle(deck);
	updatehands_server(hands, deck, hit);
	// for(i=0; i < num - 1; i++)
	res = write(fd, hit, SIZE_HIT);
	if (res < SIZE_HIT) {
		printf("Write failed with errno %d\n", errno);
		res = -1;
		goto exit;
	}
	hit[0] = -1; // Dealer starts with only 1 card revealed
	for(i = 1; i < MAX_PLAYERS_PER_GAME; i++)
		hit[i] = 1;
	updatehands_server(hands, deck, hit); // Create initial hand
	// for(i=0; i < num - 1; i++)
	res = write(fd, hit, SIZE_HIT);
	if (res < SIZE_HIT) {
		printf("Write failed with errno %d\n", errno);
		res = -1;
		goto exit;
	}
exit:
	return res;
}

// Function called at beginning of game
void playgame(void *newsock)	{

	int fd = *(int *)newsock;
	deck_t *deck = (deck_t *)malloc(sizeof(*deck));
	deck->len = DECKSIZE;
	hand_t hands[MAX_PLAYERS_PER_GAME]; // Element 0 is the dealer, other players are higher numbers 
	int done = 0, res; 
	int hit[MAX_PLAYERS_PER_GAME + 1];

	assert(hit!=NULL);

	res = start(fd, hands, deck, hit);
	if(res == -1)
		goto exit;

	// Actually play the game
	while(!done)
	{
		res = read(fd, &hit[1], sizeof(int));
		if(res <= 0)	{
			goto exit;
		}
		if(hit[1] == 2) // expand to all players in future
			done = 1; // All players have finished
		else
		{
			updatehands_server(hands, deck, hit);
			res = write(fd, hit, sizeof(hit));
			if (res < sizeof(hit)) {
				printf("Write failed with errno %d\n", errno);
				res = -1;
				goto exit;
			}
			if(hands[1].value > 21) { // Player busted
				hit[1] = -1;
				done = 1;
			}
		}
	}
	// Server plays now
	while(hands[0].value < 17)	{
		hit[0] = 1;

		updatehands_server(hands, deck, hit);
		if(hands[0].value >= 17)
			hit[MAX_PLAYERS_PER_GAME] = 1;
		res = write(fd, hit, sizeof(hit));
		if (res < sizeof(hit)) {
			printf("Write failed with errno %d\n", errno);
			res = -1;
			goto exit;
		}
	}
exit:
	free(deck);
	close(fd);
	pthread_exit((void *)&fd);
}

void updatehands_server(hand_t *hands, deck_t *deck, int *hit)
{
	int i, card;

	for(i = 0; i < MAX_PLAYERS_PER_GAME; i++)
	{
		printf("len=%d\n", deck->len);
		assert(deck->len>=0);
		if(hit[i] == 1)
		{
			deck->len--;
			card = deck->cards[deck->len];
			deck->cards[deck->len] = -1; // Not necessary, but nice to see if something odd is happening
			newhandval(&hands[i], card);
			hit[i] = card;
		}
		else
			hit[i] = -1;
	}
}
