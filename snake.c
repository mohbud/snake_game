#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>
#include <time.h>

/*Constants for maximum screen width/height and the snakes starting position
at the midpoint of the screen*/
#define SCREENWIDTH COLS
#define STARTX COLS/2
#define SCREENHEIGHT LINES
#define STARTY LINES/2

int snakeLength = 3;
int score = 0;
int speed = 20;
int winflg = 0;

/* Creat structure for trophies */
typedef struct trophies {
    int X;
    int Y;
    int number;
    double limit;
    clock_t t;
}trophies;

int kbhit (void) {
    /*checks for user inputed direction via key press*/
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec=0;
    tv.tv_usec=0;
    FD_ZERO(&read_fd);
    FD_SET(0,&read_fd);

    if(select(1, &read_fd, NULL, NULL, &tv) == -1){
        return 0;
    }

    if(FD_ISSET(0,&read_fd)){
        return 1;
    }

    return 0;
}

/*function moves snake one unit forward and check for user inputed direction*/
void movement(int *snakePosition, int dirX, int dirY, int speed, int snakeLength) {
    int prevX = 0;
    int prevY = 0;
    int curX = *snakePosition;
    int curY = *(snakePosition+1);
    int nextX = curX + dirX;
    int nextY = curY + dirY;

    /*moves snake's head to new position */
    *(snakePosition) = nextX;
    *(snakePosition+1) = nextY;
    mvprintw(nextY, nextX, "O");

    /*moves each snakePosition to the position of the next*/
    for (size_t i = 2; i < snakeLength*2; i += 2) {
        prevX = curX;
        prevY = curY;
        curX = *(snakePosition+i);
        curY = *(snakePosition+i+1);
        nextX = prevX;
        nextY = prevY;
        *(snakePosition+i) = nextX;
        *(snakePosition+i+1) = nextY;

        mvprintw(nextY, nextX, "O");
    }

    /*controls snake speed
    slows down if snake is traveling up or down*/
    if (dirY != 0) {
        usleep(500000/speed);
    }
    usleep(1000000/speed);
}

/* Function to make trophies */
void trophy_generate(trophies *trophy) {
    /* Make array for 1 - 9 */
	int ch[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    /* Generate random coordinates for trophy */
    srand(time(0));
    trophy->X = (rand() % (SCREENHEIGHT - 5) + 1);
    srand(time(0));
    trophy->Y = (rand() % (SCREENWIDTH - 5) + 1);
    
    /* Generate random trophy value and time */
    trophy->number = ch[rand() % 9];
    trophy->limit = ch[rand() % 9];
    trophy->t = clock(); 
}

/* Function to print trophies */
void trophy_print(trophies *trophy) {
  	mvprintw(trophy->X, trophy->Y, "%d", trophy[0].number);
}

/* Function to check if trophy is eaten. If true,
   increase size, speed, and score */
int eatTrophy(int *snakePosition, trophies *trophy) {
    if ((*(snakePosition) == trophy->Y) && (*(snakePosition + 1) == trophy->X)) {
		snakeLength += trophy->number;
        score += trophy->number;
        speed += trophy->number;
        trophy->X = 0;
        trophy->Y = 0;
        return 1;
    } 

    /* If user does not eat trophy in time, generate new one */
    else if((clock()>(trophy->t + (trophy->limit*1000)))){
        trophy->X = 0;
        trophy->Y = 0;
        return 1;
    }
    return 0;
}

/*function checks if snake is dead*/
int death(int *snakePosition, int snakeLength) {
    /*this checks if the snake hits any of the boundaries of the pit*/
    int x = *snakePosition;
    int y = *(snakePosition+1);

    if (x < 1 || x > (SCREENWIDTH-2)) {
        return 1;
    }
    else if (y < 1 || y > (SCREENHEIGHT-2)) {
        return 1;
    }

    /*this checks if the snake hit itself */
    for (size_t i = 2; i < snakeLength*2+2; i+=2) {
        if (x == *(snakePosition+i) && y == *(snakePosition+i+1)) {
            return 1;
        }
    }
    return 0;
}

int main() {

    /*Declaration and initialization of array to hold snakes body
    maximum size is currently 100 parts, but can be changed
    to maximum number of available cells later*/
    int snakePosition[100][2];
    for (size_t i = 0; i < 100; i++) {
        for (size_t j = 0; j < 2; j++) {
            snakePosition[i][j] = 0;
        }
    }

    int keyInput = 0;   /*new direction inputed y the user*/

    /* Randomly start direction */
    int ch[] = {1, -1, 0};
    srand(time(NULL));
    int dirX = ch[rand() % 3];
    int dirY;
    if (dirX == 0) {
		dirY = ch[rand() % 2];
    }
    else {
		dirY = 0;
    }    
    
    initscr();
    trophies trophy;                    /* Create trophy structure */
    trophy_generate(&trophy);           /* Initialize trophies */

    snakePosition[0][0] = STARTX; /*This is the snakes starting position in the pit*/
    snakePosition[0][1] = STARTY;

    curs_set(false);
    noecho();
    /*bool bf = true;*/
    keypad(stdscr, TRUE);
    
    /*loop to play game until snake dies*/
    while (!death(&snakePosition[0][0], snakeLength)) {
        erase();
        
        /*Make the snake pit */
        for(int i = 0; i<LINES; i++){
            if(i==0 || i==(LINES-1)){
                for(int j = 0; j<COLS; j++){
                    mvprintw(i,j,"-");
                }
            }
            else{
                mvprintw(i,0,"|");
                mvprintw(i,(COLS-1),"|");
            }
        }

        trophy_print(&trophy);  /* Print trophy */

        /*move the snake*/
        movement(&snakePosition[0][0], dirX, dirY, speed, snakeLength);
        refresh();
        
        /* Make new trophy if current one is eaten */
        if (eatTrophy(*snakePosition, &trophy)) {
			trophy_generate(&trophy);
			trophy_print(&trophy);
        }

        /*use arrow keys to change direction*/
        if (kbhit()) {
            keyInput = getch();

            if (keyInput == KEY_UP && !(dirY == 1 && dirX == 0)) {
                dirY = -1;
                dirX = 0;
            }
            if (keyInput == KEY_DOWN && !(dirY == -1 && dirX == 0)) {
                dirY = 1;
                dirX = 0;
            }
            if (keyInput == KEY_LEFT && !(dirY == 0 && dirX == 1)) {
                dirY = 0;
                dirX = -1;
            }
            if (keyInput == KEY_RIGHT && !(dirY == 0 && dirX == -1)) {
                dirY = 0;
                dirX = 1;
            }

            /* If user tries to go in opposite direction, snake dies */
            if (keyInput == KEY_UP && dirY == 1) {
                break;
            }
            if (keyInput == KEY_DOWN && dirY == -1) {
                break;
            }
            if (keyInput == KEY_LEFT && dirX == 1) {
                break;
            }
            if (keyInput == KEY_RIGHT && dirX == -1) {
                break;
            }

            /* If user snakeLength reaches half of perimeter, user wins */
            if (snakeLength >= ((SCREENHEIGHT * 2) + (SCREENWIDTH * 2)) / 2) {
                winflg = 1;
                break;
            }
        }
    }
    erase();
    if (winflg == 1) {
        mvprintw(SCREENHEIGHT/2,SCREENWIDTH/2,"You Won! - Your score was: %d", snakeLength);
    }
    else {
      mvprintw(SCREENHEIGHT/2,SCREENWIDTH/2,"Game Over! - Your score was: %d", snakeLength);
    }
    refresh();
	getch();			/* Wait for user input */
	endwin();			/*end game*/
	return 0;
}
