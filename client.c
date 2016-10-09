#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <fcntl.h>
#include <signal.h>

#include "cards.h"

#define PRINTF(fmt, ...) \
	printf("%s:%d: " fmt, __func__, __LINE__, ## __VA_ARGS__)

sig_atomic_t sockfd; // Needed for alarm handling

void catch_alarm(int sig)
{
	close(sockfd);
	sockfd = -1;
	printf("Response not sent quickly enough, game ended.\n");
	printf("Thank you for playing our Blackjack game!\n");
	signal(sig, catch_alarm);
}


void play_game()
{
	hand_t hands[MAX_PLAYERS_PER_GAME]; // Element 0 is the dealer, element 1 is the player, other players are higher numbers 
	int i, j, done, update[MAX_PLAYERS_PER_GAME + 1];
	ssize_t res;
	// initialize everything
	for(i = 0; i < MAX_PLAYERS_PER_GAME; i++)   {
		for(j = 0; j < 20; j++)
			hands[i].contents[j] = 0;
		hands[i].num_cards = 0;
		hands[i].value = 0;
		hands[i].num_aces = 0;
		update[i] = -1;
	}
	update[MAX_PLAYERS_PER_GAME] = 0;
	signal(SIGALRM, catch_alarm);

	printf("Waiting for game to begin...\n");
	// Create initial hands
	if ((res = read(sockfd, update, sizeof(update))) > 0)
	{
	        // PRINTF("after read %lu, res %ld\n", sizeof(update), res);
		// read in numplayers in future scope
		// also check that hand is legal
		if(res < sizeof(update)) {
			PRINTF("Read failed with errno %d\n", errno);
			goto exit;
		}
		updatehands(hands, update);
		res = read(sockfd, update, sizeof(update));
		if(res < sizeof (update)) {
			PRINTF("Read failed with errno %d\n", errno);
			goto exit;
		}
		updatehands(hands, update);

		/* while the player HITs */
		res = 0;
		while (1)
		{
			display(hands);
			int choice;

			printf("\n");
			printf("1. Hit\n");
			printf("2. Stand\n");
			printf("Please choose 1 or 2: ");
			fflush(stdout);

			alarm(TIMEOUT);
			scanf("%d", &choice);
			if(choice != 1 && choice != 2)
			{
				printf("Please give legal input.\n");
				continue;
			}
			res = write(sockfd, &choice, sizeof(choice));
			alarm(0);
			if (res < sizeof(choice)) {
				PRINTF("Write failed with errno %d\n", errno);
				res = -1;
				break;
			}
			if(choice == 2)  { // Once standing, must stand
				break;
			}
			// PRINTF("before read\n");
			res = read(sockfd, update, sizeof(update));
			if (res < sizeof(choice)) {
				PRINTF("read failed with errno %d\n", errno);
				res = -1;
				break;
			}
			// PRINTF("after read\n");
			updatehands(hands, update);
			if(hands[1].value > 21) { // if you busted
				break;
			}
		}
		if (res == -1) {
			goto exit;
		}
	        // PRINTF("after if read, res\n");
		// Let the dealer and the rest of the players finish, but continue to watch
		// PRINTF("enter while\n");
		done = 0;
		while (done == 0)
		{
			// PRINTF("done=%d\n", done);
			res = read(sockfd, update, sizeof(update));
			if (res < sizeof(update)) {
				PRINTF("read failed with errno %d\n", errno);
				res = -1;
				break;
			}
			updatehands(hands, update);
			done = update[MAX_PLAYERS_PER_GAME];
		}
		if (res == -1) {
			goto exit;
		}

		// The scoring; probably want to rework the logic and mention specifically what happens to player i.

		display(hands);
		for(i = 1; i < MAX_PLAYERS_PER_GAME; i++)	{
			//basic comparison
			if(hands[i].value > 21)
				printf("Player %d busts!\n", i);
				// player busts, write something here
			else if(hands[0].value > 21)
				printf("Dealer busted, you win!\n");
			else if(hands[i].value < hands[0].value)
				printf("Player %d loses!\n", i);
				// player loses, write something here
			else if(hands[i].value > hands[0].value)	{
				printf("Player %d wins!\n", i);
				// player wins
			}
			else  { 
			// values are equal. 
			// Additional calculations needed in the case of a 21 because
			// if only one of the player or a dealer has a blackjack
			// (A + 10), that player wins; otherwise, they draw.
				if(hands[i].value != 21)   {
					printf("Player %d draws!\n", i);
					// player ties
				}
				// if players have 21s; check which, if any, have a blackjack. Rework maybe?
				else {
					int player_blackjack = 0, dealer_blackjack = 0;
					if(hands[i].num_cards == 2)
						player_blackjack = 1;
					if(hands[i].num_cards == 2)
						dealer_blackjack = 1;
					if(player_blackjack == dealer_blackjack)
						printf("Player %d draws!\n", i);
					else if(player_blackjack > dealer_blackjack)
						printf("Player %d wins!\n", i);
					else
						printf("Player %d loses!\n", i);
				}
			}
		}
	}
exit:
	printf("Thanks for playing with us! Please consider playing again!\n");
	close(sockfd);
}

int main(int argc, char** argv)
{
	char *ip_addr;
	int port;
	struct sockaddr_in serverAddress;

	if (argc != 3)
	{
		printf("Usage: ./client ip_address port_no\n");
		printf("Example ./client 127.0.0.1 5000\n");
		exit(0);
	}

	ip_addr = argv[1];
	port = atoi(argv[2]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(ip_addr);
	serverAddress.sin_port = htons(port);
	memset(serverAddress.sin_zero, '\0', sizeof (serverAddress.sin_zero));

	if (0 > connect(sockfd, (const struct sockaddr *) &serverAddress, sizeof (serverAddress)))
		errexit("Error. Cannot connect to server.");
	else
	{
		play_game(sockfd);
		close(sockfd);
	}
	printf("\nGoodbye...\n");

	return (0);
}

void display(hand_t *hands)	{
	// This function takes in the hands and displays them in a readable fornat. Player 0 is the dealer, player 1 is the client, player 2 and up are the other players in the game 
	int i;

	// The dealer
	printf("\nThe dealer has: ");
	displayplayer(&hands[0]);

	// The client
	printf("\nI have: ");
	displayplayer(&hands[1]);
	if(hands[1].value > 21)
		printf("I busted with a %d!\n", hands[1].value);

	// Other players
	for(i = 2; i < MAX_PLAYERS_PER_GAME; i++)   {
		printf("\nPlayer %d has: ", i);
		displayplayer(&hands[i]);
		if(hands[i].value > 21)
			printf("Player %d busted with a %d!\n", i, hands[i].value);
	}
}

void displayplayer(hand_t *hand)  {
	int i;

	for(i = 0; i < hand->num_cards; i++)
		printf("%s, ", cards[hand->contents[i]].name);
	printf("\nWith a value of %d\n", hand->value);
}

void updatehands(hand_t *hands, int *update)  {
	int i;

	for(i = 0; i < MAX_PLAYERS_PER_GAME; i++)	{
		printf("update[%d]=%d\n", i, update[i]);
		if(update[i] >= 0)  { // An update was sent for that player
			newhandval(&hands[i], update[i]);
		}
	}
}
