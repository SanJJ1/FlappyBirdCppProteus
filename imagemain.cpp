#include "FEHLCD.h"
#include "FEHSD.h"
#include "FEHUtility.h"
#include <math.h>
#include <stdio.h>
// #include <iostream>
// using namespace std;
bool collisionBuffer[240][320];
class Image
{
    public:
        Image(const char *);
        void display(int, int);
        bool addCollision(int,int);
        void clearCollisions();
    private:
        unsigned int *data;
        const char *fn;
        int w, h;
};


Image::Image(const char *fname)
{
    fn = fname;
    FILE *fptr = fopen(fname, "r");
    fscanf(fptr, "%i", &w);
    fscanf(fptr, "%i", &h);
    data = new unsigned int[w * h];

    for (int i = 0; i < w * h; i ++)
    {
        fscanf(fptr, "%ui", &data[i]);

    }
    fclose(fptr);

    // // These next 4 lines are not necessary; they simply set the four corners 
    // // of the image to blue pixels, just to see the border of each image.
    // data[0] = 0xFF0000FF;
    // data[w - 1] = 0xFF0000FF;
    // data[(h - 1) * w ] = 0xFF0000FF;
    // data[w * h - 1] = 0xFF0000FF;
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
                LCD.SetFontColor(col&0xffffff);
                LCD.DrawPixel(x + i, y + j);
            }
        }
    }
}

//check if the image collides with other selected images
bool Image::addCollision(int x,int y){
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {   
            //get the 32 bit color of the pixel
            unsigned int col = data[j * w + i];
            //if the pixel is on the screen and not transparent go on
            if (col & 0xFF000000 && y + j < 240 && y + j >= 0)
            {
                //if the collision buffer wasn't already filled in on this pixel, fill it in
                //if not, another pixel was here, and the shape collided with another shape, so return true
                if(!collisionBuffer[y+j][x+i])
                    collisionBuffer[y+j][x+i]=true;
                else{
                    //printf("%i,%i\n",y+j,x+i);
                    return true;
                }
            }
        }
    }
    //if no collisions were detected, return false
    return false;
}

//clear the collision buffer
void clearCollisions(){
for (int i = 0; i < 320; i++)
    {
        for (int j = 0; j < 240; j++)
        {
            collisionBuffer[j][i]=false;
        }
    }
}

int main() 
{
    //LCD.SetBackgroundColor(LCD.Blue);     // Clear background
    LCD.Clear();

    // background and foreground positions and velocities
    float bgX = 50, bgVel = 1, fgX = 240, fgVel = 2;

    // variables for the bird's mechanics.
    float y = 0, yVel = 0, g = 1.3, bounceVel = -5;

    // xTouch, yTouch; variables for detecting touch.
    int xt, yt; 
    float smooth = 0;

    Image ybdf("s/ybdf.txt");   // ybdf = yellow bird down flap
    Image ybmf("s/ybmf.txt");   // ybmf = yellow bird mid flap
    Image ybuf("s/ybuf.txt");   // ybuf = yellow bird up flap
    Image yb[] = {ybdf, ybmf, ybuf, ybmf};
    Image bg("s/bg-day.txt");
    Image base("s/base.txt");
    Image tPipe("s/pgd.txt");
    Image bPipe("s/pg.txt");

    bool alive = true;
    int fF = 0;
    while (alive) 
    {
        LCD.Update();
        LCD.Clear();

        clearCollisions();


        y += yVel;
        yVel += g;
        bgX -= bgVel;
        fgX -= fgVel;
        fF++;
        fF %= 100;
        smooth += .04;
        bg.display(bgX, 0);
        bg.display(bgX + 132, 0);
        bg.display(bgX + 264, 0);

        tPipe.display(fgX, -240);
        bPipe.display(fgX, 150);

        tPipe.addCollision(fgX, -240);
        bPipe.addCollision(fgX, 150);

        base.display(fgX, 195);
        base.addCollision(fgX, 195);


        //yb[(int) (fF / 3.0) % 4].display(50, (int) (sin(smooth) * 30) + 100);
        yb[(int) (fF / 3.0) % 4].display(50, y);
        
 
        if (LCD.Touch(&xt, &yt)/* */)
        {
            yVel = bounceVel;
        }
        if(yb[(int) (fF / 3.0) % 4].addCollision(50, y)|| y > 240 - 24 - 5){
            alive = false;
        }
        
    }
    //return 0;
}


