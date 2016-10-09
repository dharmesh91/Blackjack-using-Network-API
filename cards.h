#ifndef __CARDS__H_
#define __CARDS__H_

#include <stdint.h>

#define MAX_PLAYERS_PER_GAME 2
#define DECKSIZE 52
#define BUFFER_SIZE 2048
#define MESSSIZE 32
#define TIMEOUT 2

typedef struct card	{
	int value;
	char *name;
} card_t;

typedef struct hand	{
	int contents[20];
	int num_cards;
	int value;
	int num_aces;
} hand_t;

typedef struct deck 	{
	int cards[DECKSIZE];
	int len;
} deck_t;

void updatehands(hand_t *hands, int *update);
void newhandval(hand_t *hand, int newcard);
void display(hand_t *hands);
void displayplayer(hand_t *hand);
void errexit(const char *, ...);
void shuffle(deck_t *deck);
void playgame(void *newsock);
void updatehands_server(hand_t *hands, deck_t *deck, int *hit);
int start(int fd, hand_t *hands, deck_t *deck, int *hit);

const static card_t cards[] = {
	{ 11, "Ace of Hearts"},
	{ 11, "Ace of Spades"},
	{ 11, "Ace of Clubs"},
	{ 11, "Ace of Diamonds"},
	{ 2, "Two of Hearts"},
	{ 2, "Two of Spades"},
	{ 2, "Two of Clubs"},
	{ 2, "Two of Diamonds"},
	{ 3, "Three of Hearts"},
	{ 3, "Three of Spades"},
	{ 3, "Three of Clubs"},
	{ 3, "Three of Diamonds"},
	{ 4, "Four of Hearts"},
	{ 4, "Four of Spades"},
	{ 4, "Four of Clubs"},
	{ 4, "Four of Diamonds"},
	{ 5, "Five of Hearts"},
	{ 5, "Five of Spades"},
	{ 5, "Five of Clubs"},
	{ 5, "Five of Diamonds"},
	{ 6, "Six of Hearts"},
	{ 6, "Six of Spades"},
	{ 6, "Six of Clubs"},
	{ 6, "Six of Diamonds"},
	{ 7, "Seven of Hearts"},
	{ 7, "Seven of Spades"},
	{ 7, "Seven of Clubs"},
	{ 7, "Seven of Diamonds"},
	{ 8, "Eight of Hearts"},
	{ 8, "Eight of Spades"},
	{ 8, "Eight of Clubs"},
	{ 8, "Eight of Diamonds"},
	{ 9, "Nine of Hearts"},
	{ 9, "Nine of Spades"},
	{ 9, "Nine of Clubs"},
	{ 9, "Nine of Diamonds"},
	{ 10, "Ten of Hearts"},
	{ 10, "Ten of Spades"},
	{ 10, "Ten of Clubs"},
	{ 10, "Ten of Diamonds"},
	{ 10, "Jack of Hearts"},
	{ 10, "Jack of Spades"},
	{ 10, "Jack of Clubs"},
	{ 10, "Jack of Diamonds"},
	{ 10, "Queen of Hearts"},
	{ 10, "Queen of Spades"},
	{ 10, "Queen of Clubs"},
	{ 10, "Queen of Diamonds"},
	{ 10, "King of Hearts"},
	{ 10, "King of Spades"},
	{ 10, "King of Clubs"},
	{ 10, "King of Diamonds"}  };

#endif //cards.h
