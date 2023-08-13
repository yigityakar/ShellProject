#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int n = 5;
void print_field(char field[n][n]){
    printf("\n----------Field----------\n");
    for (int i = 0; i < n; i ++){
        printf("\t");
        for (int j = 0; j < n; j ++){
            printf("%c ", field[i][j]);
        }
        
        printf("\n");
    }
    printf("-------------------------\n");
}
void print_mines(int mines[n][n]){
    printf("\n-----------Mines----------\n");
    for (int i = 0; i < n; i ++){
        printf("\t");
        for (int j = 0; j < n; j ++){
            printf("%d ", mines[i][j]);
        }
        printf("\n");
    }
}
void display_game_info(){
    printf("Welcome to MineZone!\n");
    printf("In this game, you should pass through a minefield of size 5x5 to claim the treasure, which is determined by how high you score!\n");
    printf("Your position is marked as '>' and the mines are labeled 'X'. The goal is to reach '$', where the treasure awaits.\n");
    printf("You can move left/right/up or down. If you step on a mine, you lose twice the score you could get for that move.\n");
    printf("Be careful! You are competing against time. Every second you wait decreases the score you get!\n");

    printf("\n");
    printf("There are three game modes that you can choose from:\n");
    printf("\teasy:\t5 mines\n\tmedium:\t10 mines\n\thard:\t15 mines\n");
    printf("\n");
}   
int main(int argc, char *argv[]){
    
    display_game_info();
    
    char user_mode[10];
    int mode = -1;

    while(mode == -1){
        printf("Enter the game mode: ");       
        fgets(user_mode, 10, stdin);

        int i = strlen(user_mode) -1;
        if (user_mode[i] == '\n'){
            user_mode[i] = '\0';
        }

        if (strcmp("easy",user_mode)==0){
            mode = 0;
            break;
        } else if (strcmp("medium",user_mode)==0){
            mode = 1;
            break;
        } else if (strcmp("hard",user_mode)==0){
            mode = 2;
            break;
        } else {
            printf("You entered an invalid mode. Choose between: easy/medium/hard.\n");
        }
    }

    char field[n][n];
    for (int i = 0; i < n; i ++){
        for (int j = 0; j < n; j ++){
            field[i][j] = 'o';
        }
    }
    field[n-1][0] = '>';
    field[0][n-1] = '$';

    /* create the mines */
    /* number of mines created in each mode: */
    /* easy     :   5 */
    /* medium   :   10 */
    /* hard     :   15 */
    
    int x,y;
    srand(time(0));
    for (int m = 0; m < n*(mode + 1); m++){
        x = rand() % n;
        y = rand() % n;
        while ((x == 0 && y == n-1) | (x == n-1 && y == 0)| (field[x][y] == 'X')){
            x = rand() % n;
            y = rand() % n;
        }
        field[x][y] = 'X';
    }

    time_t seconds;
    int start_time = time(NULL);
    int prev_time = start_time;
    int score = 0;
    int success_score = 50; // decrements as seconds pass
    int lose = 0;
    int mines_stepped = 0;

    int pos[2]; // position of the user - the user starts at leftmost corner, corresponding to n-1'th row, 0'th column
    pos[0] = n-1;   // row index
    pos[1] = 0;     // column index
    while (lose == 0){
        print_field(field);
        printf("Score: %d\n", score);
        printf("Enter next move (l:left, r:right, u:up, d:down): ");
        
        char moves[5]; 
        fgets(moves, 5, stdin);
        int curr_time = time(NULL);
        success_score -= (curr_time - prev_time); // decrease the success score by the time difference between moves
        prev_time = curr_time;
        char move = moves[0];

        if (move == 'l'){           // left
            if (pos[1] == 0){ 
                printf("You cannot move left at this position.\n");
                continue;
            }
            /* move left */
            field[pos[0]][pos[1]] = ' ';
            pos[1] --;

            /* check for mines */
            if (field[pos[0]][pos[1]] == 'X'){ 
                printf("You stepped on a mine!\n");
                mines_stepped += 1;
                field[pos[0]][pos[1]] = '<';
                score -= (2*success_score);
                continue;
            }
            field[pos[0]][pos[1]] = '<';
            score += success_score;

        } else if (move == 'r'){    // right
            if (pos[1] == n-1){ 
                printf("You cannot move right at this position.\n");
                continue;
            }
            /* move right */
            field[pos[0]][pos[1]] = ' ';
            pos[1]++;

            /* check for the goal */
            if (field[pos[0]][pos[1]] == '$'){ 
                field[pos[0]][pos[1]] = '>';
                lose = -1;
            }

            /* check for mines */
            if (field[pos[0]][pos[1]] == 'X'){
                printf("You stepped on a mine!\n");
                mines_stepped += 1;
                field[pos[0]][pos[1]] = '>'; 
                score -= (2*success_score);
                continue;
            }
            field[pos[0]][pos[1]] = '>';
            score += success_score;

        } else if (move == 'u'){    // up
            if (pos[0] == 0){ 
                printf("You cannot move up at this position.\n");
                continue;
            }
            /* move up */
            char curr = field[pos[0]][pos[1]];
            field[pos[0]][pos[1]] = ' ';
            pos[0]--;

           /* check for the goal */
            if (field[pos[0]][pos[1]] == '$'){ 
                field[pos[0]][pos[1]] = curr;
                lose = -1;
            }

            /* check for mines */
            if (field[pos[0]][pos[1]] == 'X'){ 
                printf("You stepped on a mine!\n");
                mines_stepped += 1;
                field[pos[0]][pos[1]] = curr;
                score -= (2*success_score);
                continue;
            }
            field[pos[0]][pos[1]] = curr;
            score += success_score;

        } else if (move == 'd'){    // down
            if (pos[0] == n-1){ 
                printf("You cannot move down at this position.\n");
                continue;
            }
            /* move down */
            char curr = field[pos[0]][pos[1]];
            field[pos[0]][pos[1]] = ' ';
            pos[0]++;

            /* check for mines */
            if (field[pos[0]][pos[1]] == 'X'){ 
                printf("You stepped on a mine!\n");
                mines_stepped += 1;
                field[pos[0]][pos[1]] = curr;
                score -= (2*success_score);
                continue;
            }
            field[pos[0]][pos[1]] = curr;
            score += success_score;

        } else {
            printf("You entered an invalid move. Choose between: l/r/u/d.\n");
        }

    }
    print_field(field);
    if (lose == 1) { 
        printf("GAME OVER!\n");

    } else { 
        printf("YOU WON!\n");
        
    } 
    printf("Number of mines you stepped on: %d\n", mines_stepped);
    printf("Score: %d \n", score);
    return 0;
 
}