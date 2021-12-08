
#include "FEHLCD.h"
#include "FEHSD.h"
#include "FEHUtility.h"
#include "FEHRandom.h"
// #include <stdio.h>
#include <math.h>
// #include <iostream>
// using namespace std;

#define WIDTH 320
#define HEIGHT 240
#define PIPE_NUM 2
#define PIPE_WIDTH WIDTH/PIPE_NUM
#define GAP_HEIGHT_RANGE 60
#define GAP_SIZE 80
#define GAP_CONSTANT 40 

// xTouch, yTouch; variables for detecting touch.
int xt = 0, yt = 0, status = 0, highscore = 1234567890;
float score = 0;
bool collisionBuffer[240][320] = {false}, active = true;

void clearCollisions();

class Image
{
    public:
        Image(const char *);
        ~Image();
        void display(int, int);
        bool addCollision(int,int);
    protected:
        unsigned int *data;
        const char *fn;
        int w, h;
};

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

class Pipe{
    public:
        Pipe(float);
        Pipe(){};
        void update(float);
        void display();
        float x,y;
    private:
        Image top = Image("s/pgd.txt") , bottom = Image("s/pg.txt");
        bool gavePoints = false;
};

void restart();
void start();
void collision();
void backFunction();
void showStats();
void showCredits();
void quitFunction();
void displayScore(int, int, int, int, Image*);
void clearCollisions();


Pipe pipes[PIPE_NUM];


int main() 
{
    //LCD.Clear();

    // background and foreground positions and velocities
    float bgX = 240, bgVel = 1, fgX = 240, fgVel = 2;

    // variables for the bird's mechanics.
    float y = 0, yVel = 0, g = .4, bounceVel = -3;

    float smooth = 0;

    Image yb[] = {
        Image("s/ybdf.txt"), Image("s/ybmf.txt"), Image("s/ybuf.txt"),
        Image("s/ybmf.txt")
        },
        bigNums[] = {
            Image("s/0.txt"), Image("s/1.txt"), Image("s/2.txt"),
            Image("s/3.txt"), Image("s/4.txt"), Image("s/5.txt"),
            Image("s/6.txt"), Image("s/7.txt"), Image("s/8.txt"),
            Image("s/9.txt")
        },
        medNums[] = {
            Image("s/0m.txt"), Image("s/1m.txt"), Image("s/2m.txt"),
            Image("s/3m.txt"), Image("s/4m.txt"), Image("s/5m.txt"),
            Image("s/6m.txt"), Image("s/7m.txt"), Image("s/8m.txt"),
            Image("s/9m.txt")
        },
        smallNums[] = {
            Image("s/0s.txt"), Image("s/1s.txt"), Image("s/2s.txt"),
            Image("s/3s.txt"), Image("s/4s.txt"), Image("s/5s.txt"),
            Image("s/6s.txt"), Image("s/7s.txt"), Image("s/8s.txt"),
            Image("s/9s.txt")
        },
        medals[] = {
            Image("s/plat.txt"), Image("s/gold.txt"), Image("s/silv.txt"),
            Image("s/bron.txt")
        },
    bg("s/bg-day.txt"), base("s/base3.txt"),
    gmeOvr("s/go.txt"), statsContent("s/statc.txt"), creditsContent("s/credc4.txt");

    //initialize positions of pipes
    for(int i=0;i<PIPE_NUM;i++){
        pipes[i].x = WIDTH+(i*PIPE_WIDTH);
        pipes[i].y = Random.RandInt()%(GAP_HEIGHT_RANGE+1);
    }

    // Buttons on start page
    Button play("s/m.txt", 114, 25, &start);

    // Buttons on game-over page
    Button replay("s/play.txt", 214, 100, &restart);
    Button stats("s/stats.txt", 134, 100, &showStats);
    Button credits("s/credits.txt", 54, 100, &showCredits);
    Button quit("s/quit.txt", 134, 157, &quitFunction);

    // Buttons on stats pages
    Button backStats("s/back.txt", 20, 20, &backFunction);
    Button backCredits("s/credx.txt", 10, 10,&backFunction);

    float fF = 0;

    while (active)
    {
        clearCollisions();  
        LCD.Clear();
        bg.display(bgX, 0);
        bg.display(bgX + 132, 0);
        bg.display(bgX + 264, 0);

        bgX -= bgVel;
        fgX -= fgVel;
        if(fgX < 0)
        {
            fgX = 320;
        }
        if(bgX < 0)
        {
            bgX = 320;
        }
        fF += .3;
        if(fF > 4)
        {
            fF -= 4;
        }

        switch (status)
        {
            case 0:    // Start menu
                smooth += .04;
                y = (int) (sin(smooth) * 30) + 100;
                base.display(fgX, 195);
                yb[(int) fF].display(50, y);
                play.display(play.x, play.y);
                play.update();
                break;

            case 1:    // Game in progress 
                // score += 1;
                displayScore((int) score, 100, 30, 14, medNums);

                y += yVel;
                yVel += g;

                for(int i=0;i<PIPE_NUM;i++){
                    pipes[i].display();
                    pipes[i].update(fgVel);
                }

                base.display(fgX, 195);
                yb[(int) fF].display(50, y);

        
                if (LCD.Touch(&xt, &yt))
                {
                    yVel = bounceVel;
                }

                if(yb[(int) fF].addCollision(50, y) || y > 195 - 24){
                    yVel = 0;
                    collision();
                }
                break;
            
            case 2:    // Game over
                gmeOvr.display(64, 20);
                replay.display(replay.x, replay.y);
                stats.display(stats.x, stats.y);
                credits.display(credits.x, credits.y);
                quit.display(quit.x, quit.y);

                replay.update();
                stats.update();
                credits.update();
                quit.update();
                break;
            case 3:  // Stats page
                backStats.display(backStats.x, backStats.y);
                statsContent.display(50, 50);
                medals[(int) fF].display(76, 92);
                displayScore((int) score, 243, 85, 14, medNums);
                displayScore((int) highscore, 243, 125, 14, medNums);

                backStats.update();
                break;
            case 4: // Credits Page
                creditsContent.display(0, 0);
                backCredits.display(backCredits.x, backCredits.y);

                backCredits.update();
                break;
        }
    }
    
}
Image::Image(const char *fname)
{
    // fn = fname;
    // FILE *fptr = fopen(fn, "r");
    // fscanf(fptr, "%i", &w);
    // fscanf(fptr, "%i", &h);
    // data = new unsigned int[w * h];

    // for (int i = 0; i < w * h; i ++)
    // {
    //     fscanf(fptr, "%ui", &data[i]);
    // }
    // fclose(fptr);
    fn = fname;
    FEHFile *fptr = SD.FOpen(fn, "r");
    SD.FScanf(fptr, "%i", &w);
    SD.FScanf(fptr, "%i", &h);
    data = new unsigned int[w * h];

    for (int i = 0; i < w * h; i ++)
    {
        SD.FScanf(fptr, "%ui", &data[i]);
    }
    SD.FClose(fptr);
}

Image::~Image()
{
    delete data;
}

void Image::display(int x, int y)
{
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            unsigned int col = data[j * w + i];
            if (col & 0xFF000000 && y + j < 240 && y + j >= 0)
            {
                // LCD.SetFontColor(0xFFFFFFFF - col);
                LCD.SetFontColor(col);
                LCD.DrawPixel(x + i, y + j);
            }
        }
    }
    //data[0] = 0xFF0000FF;
    //data[w - 1] = 0xFF0000FF;
    //data[(h - 1) * w ] = 0xFF0000FF;
   // data[w * h - 1] = 0xFF0000FF;
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
            if (col & 0xFF000000 && y + j < 240 && y + j >= 0 && x + i < 320 && x + i >= 0)
            {
                //if the collision buffer wasn't already filled in on this pixel, fill it in
                //if not, another pixel was here, and the shape collided with another shape, so return true
                if(!collisionBuffer[y+j][x+i])
                {
                    collisionBuffer[y+j][x+i]=true;
                    LCD.SetFontColor(LCD.Blue);
                    LCD.DrawPixel(x + i, y + j);
                }
                else{
                    // printf("%d,%d\n",x+i,y+j);
                    return true;
                }
            }
        }
    }
    //if no collisions were detected, return false
    return false;
}

Button::Button(const char *fname, int X, int Y, void (*pressFun)()) : Image(fname)
{
    x = X;
    y = Y;
    pressFunction = pressFun;
}

bool Button::update(){
    //see if the screen is touched anywhere
    float touchX,touchY;
    bool touched = LCD.Touch(&touchX,&touchY);
    bool lastPressed = pressed;
    //check if the pixel touched is inside the button and the button isn't pressed yet
    if(touchX >= x && touchX <= x+w && touchY >= y && touchY <= y+h){
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

Pipe::Pipe(float X){
    x = X;
    y = Random.RandInt()%(GAP_HEIGHT_RANGE+1);
}

void Pipe::update(float velocity){
    //move position left by velocity
    x -= velocity;
    //if the pipe reaches the left of the screen, move it to the right of the screen
    if(x<52&&!gavePoints){
        gavePoints=true;
        score++;
    }
    if(x<=0){
        x=WIDTH;
        y = Random.RandInt()%(GAP_HEIGHT_RANGE+1);
        gavePoints=false;
    }
    //add pipes to the collision buffer

    bottom.addCollision(x-52,y+320-HEIGHT+GAP_SIZE/2.0+GAP_CONSTANT/2.0);
    top.addCollision(x-52,y-320+HEIGHT/2.0-GAP_SIZE/2.0-GAP_CONSTANT/2.0);
}

void Pipe::display(){
    if(x>=0 && x<WIDTH){
        bottom.display(x-52,y+320-HEIGHT+GAP_SIZE/2.0+GAP_CONSTANT/2.0);
        top.display(x-52,y-320+HEIGHT/2.0-GAP_SIZE/2.0-GAP_CONSTANT/2.0);
    }
}

void restart()
{
    score = 0;
    status = 0; // Sets status to start page
}

void start()
{
    status = 1; // Sets status to in-game
}

void collision()
{
    status = 2; // Sets status to game over
    for(int i=0;i<PIPE_NUM;i++){
        pipes[i].x = WIDTH+i*PIPE_WIDTH;
        pipes[i].y = Random.RandInt()%(GAP_HEIGHT_RANGE+1)-GAP_HEIGHT_RANGE/2.0;
    }
}

void backFunction()
{
    status = 2; // Sets status back to game over page, to go back from stats or credits page
}

void showStats()
{
    status = 3; // Sets status to stats page
}

void showCredits()
{
    status = 4; // Sets status to credits page
}

void quitFunction()
{
    active = false;
}

void displayScore(int score, int x, int y, int spacing, Image *nums)
{
    while(score > 0)
    {
        nums[score % 10].display(x, y);
        score /= 10;
        x -= spacing;
    }
}

//clear the collision buffer by setting all values to false
void clearCollisions(){
    for (int i = 0; i < 320; i++)
    {
        for (int j = 0; j < 240; j++)
        {
            collisionBuffer[j][i]=false;
        }
    }
}
