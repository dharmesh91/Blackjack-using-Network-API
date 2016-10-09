#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cards.h"

// The following function takes the value of the previous hand and calculates it anew when given a new card. hands are 2 element arrays with the value of the hand and the number of aces worth 11 in the hand(which should be 0 or 1 at all points except sometimes when a card is being added).
void newhandval(hand_t *hand, int newcard)  {
	
	hand->contents[hand->num_cards]=newcard;
	hand->num_cards++;
	hand->value += cards[newcard].value;
	if(cards[newcard].value == 11) // New card is an ace
		hand->num_aces++;
			// If we bust and have aces worth 11, downgrade them to being worth 1 until we either run out of aces or our hand value is no longer > 21 
	while(hand->value > 21 && hand->num_aces > 0)  {
		hand->value -= 10;
		hand->num_aces--;
	}
}
