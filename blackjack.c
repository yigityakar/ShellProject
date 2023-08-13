    #include <stdio.h> 
	#include <stdlib.h> 
	#include <unistd.h>
	#include <string.h>
	#include <time.h>

	
	int main(int argc, char *argv[]){


		int biggame=1;
		char newgame[2];
		int bank_account = 100;
		printf("!!!!WELCOME TO INFINITE DECK BLACKJACK!!!!\n");
		printf("In this version of BlackJack, the deck has infinite amount of cards!!\n");
		printf("So, the probability of picking any card is always the same!!\n");		
		printf("One additional twist: Aces are always worth 11 !!\n");		
		printf("Therefore, you may lose even BEFORE you start!\n");
		printf("Buy-ins are 100$!\n");
		printf("If you win, you get as much as you bet\n");



		printf("Good luck and have fun :))\n");
		printf("\n");
		while(biggame){
		printf("Balance: %d\n",bank_account);

		srand(time(NULL));
		int u1 = 1+rand()%13;
		int u2 = 1+rand()%13;
		int d1 = 1+rand()%13;
		int game =1;
		char c1[3];
		char c2[3];
		char cd1[3];
		int bet;
		if (u1==1){
			strcpy(c1,"A");
			u1=11;

			}
		else if(u1==11){
			u1=10;

			strcpy(c1,"J");
		}
		else if(u1==12){
			u1=10;

			strcpy(c1,"Q");
		}
		else if(u1==13){
			u1=10;

			strcpy(c1,"K");
		}
		else{
			sprintf(c1,"%d",u1);
		}

		if (u2==1){
			strcpy(c2,"A");
			u2=11;

			}
		else if(u2==11){

			strcpy(c2,"J");
			u2=10;
		}
		else if(u2==12){

			strcpy(c2,"Q");
			u2=10;

		}
		else if(u2==13){

			strcpy(c2,"K");
			u2=10;

		}
		else{
			sprintf(c2,"%d",u2);
		}

		if (d1==1){
			strcpy(cd1,"A");
			d1=11;
			}
		else if(d1==11){

			strcpy(cd1,"J");
			d1=10;
		}
		else if(d1==12){

			strcpy(cd1,"Q");
			d1=10;
		}
		else if(d1==13){

			strcpy(cd1,"K");
			d1=10;
		}
		else{
			sprintf(cd1,"%d",d1);
		}
		
		int user_sum= u1+u2;
		int dealer_sum=d1;
		int invalid_bet=1;
		while(invalid_bet){
			printf("Enter bet: ");
			scanf("%d",&bet);
			if (bet>bank_account  || bet <0 ){

				printf("Invalid bet\n");

			}
			else{
				invalid_bet=0;
			}

		}


		
		printf("Dealing cards....\n");
		sleep(3);
		printf("\n");
		printf("Dealer Cards: %s \n",cd1);
		printf("Dealer sum: %d \n",dealer_sum);
		printf("\n");
		printf("User Cards: %s , %s \n",c1,c2);
		printf("User sum: %d \n",user_sum);


		if(user_sum>21){
			printf("Unlucky with two Aces, You LOST!!!\n");
			game=0;
			bank_account -=bet;
			printf("Balance: %d\n",bank_account);
		}



		char hit[2];
		int h;
		int dn;
		while(game){
			printf("\n");
			printf("Do you want to hit or stand? (h/s):");
			scanf("%s",hit);

			if(strcmp(hit,"h")==0){

				h= 1+rand()%13;
				printf("Dealing for user.. \n");
				printf("\n");
				sleep(1);
				if (h==1){
					printf("User's new card: A\n");

					h=11;
				}
				else if (h==11){
					printf("User's new card: J\n");

					h=10;
				}
				else if (h==12){
					printf("User's new card: Q\n");

					h=10;
				}
				else if (h==13){
					printf("User's new card: K\n");

					h=10;
				}
				else{
				printf("User's new card: %d\n",h);

				}

			user_sum += h;
				printf("User sum: %d \n",user_sum);

			if(user_sum>21){
						printf("You lost!!!!\n");
						game=0;
						bank_account -=bet;
						printf("Balance: %d\n",bank_account);
						break;

			}

			}
			else if(strcmp(hit,"s")==0){

				if(dealer_sum>user_sum){

					printf("Dealer won, You Lost!!\n");
					
					game=0;
					bank_account -=bet;
					
					printf("Balance: %d\n",bank_account);
					break;

				}
				else if(dealer_sum==user_sum){

					printf("TIE!!\n");
					game=0;
					printf("Balance: %d\n",bank_account);

					break;

				}

				else{
					while(1){
					dn= 1+rand()%13;
					printf("Dealing for dealer.. \n");
					printf("\n");
					sleep(1);
				if (dn==1){
					printf("Dealer's new card: A\n");

					dn=11;
				}
				else if (dn==11){
					printf("Dealer's new card: J\n");

					dn=10;
				}
				else if (dn==12){
					printf("Dealer's new card: Q\n");

					dn=10;
				}
				else if (dn==13){
					printf("Dealer's new card: K\n");

					dn=10;
				}
				else{
				printf("Dealer's new card: %d\n",dn);

				}

				dealer_sum +=dn;
				printf("Dealer sum: %d \n",dealer_sum);


				if (dealer_sum > 21){

					printf("Dealer Lost, You WON!!!!\n");
					game =0;
					bank_account +=bet;
					printf("Balance: %d\n",bank_account);
					break;

				}
				if(dealer_sum>user_sum){

					printf("Dealer won, You Lost!!\n");
					game=0;
					bank_account -=bet;
					
					printf("Balance: %d\n",bank_account);
					break;

				}
				else if(dealer_sum==user_sum){

					printf("TIE!!\n");
					game=0;
					
					printf("Balance: %d\n",bank_account);
					break;

				}


					}

				}


			}
		}






		if (bank_account ==0){
			printf("You are out of money, game over \n");
			printf("Thank you for playing \n");
			break;

		}
		else{
		printf("\n");
		printf("Do you want to play another round (y/n): ");
		scanf("%s",newgame);
		if (strcmp(newgame,"n")==0){
			biggame=0;
			if(bank_account>100){

				printf("Congrats, you made %d dollars \n",bank_account-100);
			}
			else{
				printf("Better luck next time\n");
			}



			printf("Thank you for playing \n");
			break;
		}
		else{
			game=1;
		}
		}

		}
		
	 return 0;
	 
	 
	}
