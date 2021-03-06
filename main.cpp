/************************************************/
/*  Author:  Sanjay Janardhan   Date:  12/8/21  */
/*  Author:  Luke Seragglio     Date:  12/8/21  */
/*  File:  main.cpp                             */
/*  Instructor:  Paul Clingan                   */
/*  Section: 08:00am                            */
/************************************************/
#include "FEHLCD.h"
#include "FEHSD.h"
#include "FEHUtility.h"
#include "FEHRandom.h"

// Since the FEHSD.h library doesn't work on all computers, it may be necessary to 
// use <stdio.h> instead, as seen below.
// #include <stdio.h>

#define WIDTH 320
#define HEIGHT 240
#define PIPE_NUM 2
#define PIPE_WIDTH WIDTH / PIPE_NUM
#define GAP_HEIGHT_RANGE 67
#define GAP_SIZE 80
#define PIPE_HEIGHT GAP_SIZE / 2 + 24
#define DOUBLE_PI 6.28318530717
#define HALF_PI 1.570796326794

// xTouch, yTouch; variables for detecting touch.
int xTouch = 0, yTouch = 0, status = 0, highscore = 0;
float score = 0;

//collision buffer idea inspired by z buffer
//https://en.wikipedia.org/wiki/Z-buffering
bool collisionBuffer[HEIGHT][WIDTH] = {false}, active = true;
bool goomd = false;

/*
    Image Class
    Image: constructor
    ~Image: destructor
    display(): takes in x and y for top left corner of image and draws it
    addCollision(): adds visible parts of image with top left corner of (x,y) to collision buffer
    data: array of 32 bit color values
    fileName: file name
    w: width
    h: height
*/
class Image
{
    public:
        Image(const char *);
        ~Image();
        void display(int, int);
        bool addCollision(int,int);
    protected:
        unsigned int *data;
        const char *fileName;
        int w, h;
};

/*
    Button Class (Image subclass)
    update(): check if button is pressed, if so, call function
    x, y: coordinates of top left corner of button
    bool pressed: whether or not the button was previously pressed
    *pressFunction: pointer to function called on button press
*/
class Button : public Image
{
    public:
        Button(const char *, int, int, void (*)());
        bool update();
        int x, y;
    private:
        bool pressed;
        void (*pressFunction)();

};

/*
    Pipe Class
    Pipe(float): constructor
    Pipe(): constructor
    update(): update position and add pair of pipes to collision buffer
    display(): draw the pipes in the pair
    x,y: coordinates of top left corner
    top , bottom: images for bottom and top pipes
    gavePoints: whether or not points were earned for passing this pipe
*/
class Pipe{
    public:
        Pipe(float);
        Pipe();
        void update(float);
        void display();
        float x,y;
    private:
        Image top = Image("s/pgd.txt") , bottom = Image("s/pg.txt");
        bool gavePoints = false;
};

// Prototypes for all functions
void clearCollisions();
void restart();
void start();
void collision();
void backFunction();
void showStats();
void showCredits();
void quitFunction();
void displayScore(int, int, int, int, Image*);
void clearCollisions();
float sin(float);

//set up an array of pipes
Pipe pipes[PIPE_NUM];


int main() 
{
    // background and foreground positions and velocities
    float backgroundX = 0, backgroundVelocity = 1, foregroundX = 0, foregroundVelocity = 2;

    // variables for the bird's mechanics.
    float y = 0, yVelocity = 0, g = .4, bounceVelocity = -3;

    // variable for starting screen flappybirds sinewave bobbing
    float smooth = 0;

    // all those images
    Image yellowBird[] = { // Array of birds animation frames
        Image("s/ybdf.txt"), Image("s/ybmf.txt"), Image("s/ybuf.txt"),
        Image("s/ybmf.txt")
        },
        bigNums[] = { // Large font numbers
            Image("s/0.txt"), Image("s/1.txt"), Image("s/2.txt"),
            Image("s/3.txt"), Image("s/4.txt"), Image("s/5.txt"),
            Image("s/6.txt"), Image("s/7.txt"), Image("s/8.txt"),
            Image("s/9.txt")
        },
        medNums[] = { // Medium font numbers
            Image("s/0m.txt"), Image("s/1m.txt"), Image("s/2m.txt"),
            Image("s/3m.txt"), Image("s/4m.txt"), Image("s/5m.txt"),
            Image("s/6m.txt"), Image("s/7m.txt"), Image("s/8m.txt"),
            Image("s/9m.txt")
        },
        smallNums[] = { // Small font numbers
            Image("s/0s.txt"), Image("s/1s.txt"), Image("s/2s.txt"),
            Image("s/3s.txt"), Image("s/4s.txt"), Image("s/5s.txt"),
            Image("s/6s.txt"), Image("s/7s.txt"), Image("s/8s.txt"),
            Image("s/9s.txt")
        },
        medals[] = { // Medals
            Image("s/plat.txt"), Image("s/gold.txt"), Image("s/silv.txt"),
            Image("s/bron.txt")
        },
    background("s/bg-day.txt"), base("s/base3.txt"),
    gmeOvr("s/go.txt"), statsContent("s/statc.txt"), creditsContent("s/credc4.txt");

    //initialize positions of pipes
    for(int i=0;i<PIPE_NUM;i++){
        pipes[i].x = WIDTH+(i*PIPE_WIDTH);
        pipes[i].y = Random.RandInt() % (GAP_HEIGHT_RANGE + 1) + PIPE_HEIGHT;
    }

    // Buttons on start page
    Button play("s/m.txt", 114, 25, &start);

    // Buttons on game-over page
    Button replay("s/play.txt", 214, 100, &restart);
    Button stats("s/stats.txt", 134, 100, &showStats);
    Button credits("s/credits.txt", 54, 100, &showCredits);
    Button quit("s/quit.txt", 134, 157, &quitFunction);

    // Buttons on stats and credits pages
    Button backStats("s/back.txt", 20, 20, &backFunction);
    Button backCredits("s/credx.txt", 10, 10,&backFunction);

    float animationFrame = 0;


    // Graphics While loop
    while (active)
    {   
        //clear the collision buffer and screen
        clearCollisions();  
        LCD.Clear();

        //display the background
        background.display(backgroundX, 0);
        background.display(backgroundX + 132, 0);
        background.display(backgroundX + 264, 0);

        //move the foreground and background left
        backgroundX -= backgroundVelocity;
        foregroundX -= foregroundVelocity;

        //move the foreground to the right end of the screen if it goes to the left of the screen
        if(foregroundX < 0)
            foregroundX = WIDTH;
        

        //move the background to the right end of the screen if it goes to the left of the screen
        if(backgroundX < 0)
            backgroundX = WIDTH;
        

        //make flappy bird's animation frame progress slightly slower than the frames move
        animationFrame += .2;
        //this needs to be here because modulo doesn't work on floats
        if(animationFrame > 4)
            animationFrame -= 4;
        
        // Switch case implementing a state-driven FSM, inspired by 
        // https://en.wikipedia.org/wiki/Event-driven_finite-state_machine
        // basically checks 5 different cases, with each one being a different page.
        // the execution of a certain case may or may not change status, thus potentially
        // executing a new case, and effectively displaying a new page.
        switch (status)
        {
            case 0:    // Starting screen

                // make flappy bird move smoothly up and down, as the original game has in
                // the starting screen. Resets the smooth variable, so that it stays 
                // within the bounds of the taylor approximation. Does it at 2*pi
                // so that the animation stays at the same y value when smooth resets.
                smooth += .04;
                if (smooth > DOUBLE_PI)
                    smooth -= DOUBLE_PI;
                y = (int) (sin(smooth) * 30) + 100;

                //display the floor, bird, and play button
                base.display(foregroundX, 195);
                yellowBird[(int) animationFrame].display(50, y);
                play.display(play.x, play.y);

                //add functionality for play button
                if(LCD.Touch(&xTouch, &yTouch))
                {
                    yVelocity = bounceVelocity;
                    start();
                }
                // play.update();
                break;

            case 1:    // Game in progress 
                //projectile motion for bird
                if (y + yVelocity > 0)
                {
                    y += yVelocity;
                }
                yVelocity += g;

                //display pipes and put them in the collision buffer
                for(int i=0;i<PIPE_NUM;i++){
                    pipes[i].display();
                    pipes[i].update(foregroundVelocity);
                }

                //display the floor and bird
                base.display(foregroundX, 195);
                yellowBird[(int) animationFrame].display(50, y);

                //display the score
                displayScore((int) score, 155, 30, 14, medNums);

                //if the user taps the screen, make the bird "jump"
                if (LCD.Touch(&xTouch, &yTouch))
                    yVelocity = bounceVelocity;
                
                //if the bird has touched the pipes or floor, execute stuff inside
                if(yellowBird[(int) animationFrame].addCollision(50, y) || y > 195 - 24){
                    //end the game and make things nice for the next game
                    yVelocity = 0;
                    collision();
                }
                break;
            
            case 2:    // Game over
                //activate the EASTER EGG
                if(score==69)
                    goomd=true;

                //update high score
                if(score>highscore)
                    highscore = score;

                //display the game over text along with the four buttons
                gmeOvr.display(64, 20);
                replay.display(replay.x, replay.y);
                stats.display(stats.x, stats.y);
                credits.display(credits.x, credits.y);
                quit.display(quit.x, quit.y);
                base.display(foregroundX, 195);

                //add functionality for the buttons
                replay.update();
                stats.update();
                credits.update();
                quit.update();
                break;
            case 3:  // Stats page
                //display the background and back button
                backStats.display(backStats.x, backStats.y);
                statsContent.display(50, 50);
                base.display(foregroundX, 195);
                
                /*adds medals!
                last score:
                    0-9: no medal
                    10-19: bronze medal
                    20-29: silver medal
                    30-39: gold medal
                    40+ (excluding 69): platinum medal
                    69: cycles through all medals
                */
                if(score == 69)
                    medals[(int) animationFrame].display(76, 92);
                else if(score >= 40)
                    medals[0].display(76, 92);
                else if(score >= 10)
                    medals[4-(int)score/10].display(76, 92);

                //show the scores    
                displayScore((int) score, 243, 82, 14, medNums);
                displayScore((int) highscore, 243, 124, 14, medNums);

                //add functionality for a back button
                backStats.update();
                break;
            case 4: // Credits Page
                //shows a funne image for credits
                creditsContent.display(0, 0);
                backCredits.display(backCredits.x, backCredits.y);

                backCredits.update();
                break;
            default:
                status = 0;
        }
    }
    
}

// Constructor for Image class. takes in string of text file location, and reads in each
// line, representing an ARGB value, into a data array of size w * h.
Image::Image(const char *fname)
{   
    fileName = fname;
    //open the specified image file
    FILE *fptr = fopen(fileName, "r");

    //read in width and height of image
    fscanf(fptr, "%i", &w);
    fscanf(fptr, "%i", &h);

    //allocate memory for image data
    data = new unsigned int[w * h];

    //read in image colors
    for (int i = 0; i < w * h; i ++)
    {
        fscanf(fptr, "%ui", &data[i]);
    }

    //close the file
    fclose(fptr);

    //below is literally the same thing but with the broken FEHSD library

    // fileName = fname;
    // FEHFile *fptr = SD.FOpen(fileName, "r");
    // SD.FScanf(fptr, "%i", &w);
    // SD.FScanf(fptr, "%i", &h);
    // data = new unsigned int[w * h];

    // for (int i = 0; i < w * h; i ++)
    // {
    //     SD.FScanf(fptr, "%ui", &data[i]);
    // }
    // SD.FClose(fptr);
}

Image::~Image()
{
    delete data;
}

void Image::display(int x, int y)
{
    //iterate through all pixels in the image
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            //get the color at the specified pixel
            unsigned int col = data[j * w + i];

            //if the pixel is in the screen and not transparent, draw it
            if (col & 0xFF000000 && y + j < HEIGHT && y + j >= 0)
            {   
                //if the easter egg is activated, invert the  color
                if(goomd)
                    LCD.SetFontColor(0xFFFFFFFF - col);
                else
                    LCD.SetFontColor(col);

                //draw the pixel
                LCD.DrawPixel(x + i, y + j);
            }
        }
    }
}

//check if the image collides with other selected images
bool Image::addCollision(int x,int y){
    //go through all pixels in the image's bounding box
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {   
            //get the 32 bit color of the pixel
            unsigned int col = data[j * w + i];
            //if the pixel is on the screen and not transparent go on
            if (col & 0xFF000000 && y + j < HEIGHT && y + j >= 0 && x + i < WIDTH && x + i >= 0)
            {
                //if the collision buffer wasn't already filled in on this pixel, fill it in
                //if not, another pixel was here, and the shape collided with another shape, so return true
                if(!collisionBuffer[y+j][x+i])
                {
                    collisionBuffer[y+j][x+i]=true;
                }
                else{
                    return true;
                }
            }
        }
    }
    //if no collisions were detected, return false
    return false;
}

//button constructor
Button::Button(const char *fname, int X, int Y, void (*pressFun)()) : Image(fname)
{
    x = X;
    y = Y;
    pressFunction = pressFun;
}

//functionality for button
//makes it so that if the button is tapped, it calls a function
bool Button::update(){
    //see if the screen is touched anywhere
    float touchX,touchY;
    bool touched = LCD.Touch(&touchX,&touchY);
    bool lastPressed = pressed;
    //check if the pixel touched is inside the button and the button isn't pressed yet
    if(touchX >= x && touchX <= x + w && touchY >= y && touchY <= y + h){
        if(touched)//keep the button pressed if the screen is being tapped inside 
            pressed = true;
        else if(lastPressed)//if the button was pressed before but is no longer anymore, activate the function
            (*pressFunction)();
    }
    //check if the screen is no longer being tapped in the button
    else{
        pressed = false;
    }
    //pressed = true;
    return pressed;
}

//pipe constructor
Pipe::Pipe(float X){
    x = X;
    y = Random.RandInt() % (GAP_HEIGHT_RANGE + 1) + PIPE_HEIGHT;
}

// Default constructor
Pipe::Pipe(){
    y = Random.RandInt() % (GAP_HEIGHT_RANGE + 1) + PIPE_HEIGHT;
}

//updates position of pipe pair and adds them to collision buffer
void Pipe::update(float velocity){
    //move position left by velocity
    x -= velocity;

    //award points for moving past a pipe
    if(x < 52 && !gavePoints){
        score++;
        gavePoints = true;
    }
    
    //if the pipe reaches the left of the screen, move it to the right of the screen
    if(x <= 0){
        x = WIDTH;
        y = Random.RandInt() % (GAP_HEIGHT_RANGE + 1) + PIPE_HEIGHT;
        gavePoints = false;
    }

    //add pipes to the collision buffer
    top.addCollision(x - 52, y - 320 - GAP_SIZE / 2.0);
    bottom.addCollision(x - 52, y + GAP_SIZE / 2.0);

    // top.addCollision(x - 52, y - 320 + HEIGHT / 2.0 - GAP_SIZE / 2.0 - GAP_CONSTANT / 2.0);
    // bottom.addCollision(x - 52, y + 320 - HEIGHT + GAP_SIZE / 2.0 + GAP_CONSTANT / 2.0);
}

//draw the pipe
void Pipe::display(){
    if(x >= 0 && x < WIDTH){
        top.display(x - 52, y - 320 - GAP_SIZE / 2.0);
        bottom.display(x - 52, y + GAP_SIZE / 2.0);
        // top.display(x-52, y - 320 + HEIGHT / 2.0 - GAP_SIZE / 2.0 - GAP_CONSTANT / 2.0);
        // bottom.display(x-52, y + 320 - HEIGHT + GAP_SIZE / 2.0 + GAP_CONSTANT / 2.0);
    }
}

//function for setting up a new game
void restart()
{
    score = 0;
    status = 0; // Sets status to start page
}

//function for start button
void start()
{
    status = 1; // Sets status to in-game
}

//function that's called whenever the bird hits the pipe
void collision()
{
    status = 2; // Sets status to game over
    for(int i=0;i<PIPE_NUM;i++){
        pipes[i].x = WIDTH+i*PIPE_WIDTH;
        pipes[i].y = Random.RandInt() % (GAP_HEIGHT_RANGE + 1) + PIPE_HEIGHT;
    }
}

//function for back button
void backFunction()
{
    status = 2; // Sets status back to game over page, to go back from stats or credits page
}

//function for back button
void showStats()
{
    status = 3; // Sets status to stats page
}

//function for credits button
void showCredits()
{
    status = 4; // Sets status to credits page
}

//function for quit button
void quitFunction()
{
    active = false;
}

// shows the score on a specified part of the screen with a 
// specified font size: smallNums, medNums, or bigNums
void displayScore(int score, int x, int y, int spacing, Image *nums)
{
    if (score == 0)
        nums[0].display(x, y);
    else
    {
        while(score > 0)
        {
            nums[score % 10].display(x, y);
            score /= 10;
            x -= spacing;
        }
    }
}

//clear the collision buffer by setting all values to false
void clearCollisions(){
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            collisionBuffer[j][i]=false;
        }
    }
}

// Extremely naive implementation of sin(x) to avoid using external library.
// Uses first 7 terms of cosine Taylor series, since it has a lower error than the first 7
// terms of sine's Taylor series. 
// 7 terms is the minimum accuracy needed for the animation to stay smooth.
// Shifts input, using the equivalence sin(x) = cos(x - pi/2).
float sin(float x) {
    x = x - HALF_PI;
    return 
    1 - 
    ((x * x) / (2)) + 
    ((x * x * x * x) / (24)) - 
    ((x * x * x * x * x * x) / (720)) + 
    ((x * x * x * x * x * x * x * x) / (40320)) -
    ((x * x * x * x * x * x * x * x * x * x) / (3628800)) +
    ((x * x * x * x * x * x * x * x * x * x * x * x) / (479001600));
}
