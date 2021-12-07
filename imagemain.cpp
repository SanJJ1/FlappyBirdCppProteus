#include "FEHLCD.h"
#include "FEHSD.h"
#include "FEHUtility.h"
#include <math.h>
// #include <iostream>
// using namespace std;



// xTouch, yTouch; variables for detecting touch.
int xt = 0, yt = 0, status = 0;
float score = 0;

class Image
{
    public:
        Image(const char *);
        ~Image();
        void display(int, int);
    protected:
        unsigned int *data;
        const char *fn;
        int w, h;
};

class Button : public Image
{
    public:
        Button(const char *, int, int, void (*)());
        // Button(const char *) : Image(*) {}
        bool update();
        int x, y;
    private:
        bool pressed;
        void (*pressFunction)();

};

// void quit();
// void stats();
// void credits();
// void flappyBird();

Image::Image(const char *fname)
{
    fn = fname;
    FEHFile *fptr = SD.FOpen(fn, "r");
    SD.FScanf(fptr, "%i", &w);
    SD.FScanf(fptr, "%i", &h);
    data = new unsigned int[w * h];

    for (int i = 0; i < w * h; i ++)
    {
        SD.FScanf(fptr, "%i", &data[i]);
    }
    SD.FClose(fptr);

    // These next 4 lines are not necessary; they simply set the four corners 
    // of the image to blue pixels, just to see the border of each image.
    data[0] = 0xFF0000FF;
    data[w - 1] = 0xFF0000FF;
    data[(h - 1) * w ] = 0xFF0000FF;
    data[w * h - 1] = 0xFF0000FF;
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
}

Button::Button(const char *fname, int X, int Y, void (*pressFun)()) : Image(fname)
{
    x = X;
    y = Y;
    pressFunction = pressFun;
}

bool Button::update()
{
    //see if the screen is touched anywhere
    bool touched = LCD.Touch(&xt, &yt);
    bool lastPressed = pressed;
    //check if the pixel touched is inside the button and the button isn't pressed yet
    if(xt >= x && xt <= x+w && yt >= y && yt <= y+h){
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

void collision()
{
    status = 2;
    score = 0;
}

void start()
{
    status = 1;
}

void restart()
{
    status = 0;
}

void showScore(int score, Image *nums)
{
    int x = 100;
    while(score > 0)
    {
        nums[score % 10].display(x, 30);
        score /= 10;
        x -= 20;
    }
}

int main() 
{
    LCD.SetBackgroundColor(LCD.Blue);     // Clear background
    LCD.Clear();

    // background and foreground positions and velocities
    float bgX = 50, bgVel = 1, fgX = 50, fgVel = 2;

    // variables for the bird's mechanics.
    float y = 0, yVel = 0, g = 1.3, bounceVel = -4 * g;

    float smooth = 0;

    // Image ybdf("s/ybdf.txt");   // ybdf = yellow bird down flap
    // Image ybmf("s/ybmf.txt");   // ybmf = yellow bird mid flap
    // Image ybuf("s/ybuf.txt");   // ybuf = yellow bird up flap

    // Image ybdf("s/test2.txt");   // ybdf = yellow bird down flap
    // Image ybmf("s/test2.txt");   // ybmf = yellow bird mid flap
    // Image ybuf("s/test2.txt");   // ybuf = yellow bird up flap
    Image yb[] = {
        Image("s/ybdf.txt"), Image("s/ybmf.txt"), 
        Image("s/ybuf.txt"), Image("s/ybmf.txt")
        },
        nums[] = {
            Image("s/0s.txt"), Image("s/1s.txt"), Image("s/2s.txt"),
            Image("s/3s.txt"), Image("s/4s.txt"), Image("s/5s.txt"),
            Image("s/6s.txt"), Image("s/7s.txt"), Image("s/8s.txt"),
            Image("s/9s.txt")
        },
    bg("s/bg-day.txt"), base("s/base.txt"), tPipe("s/pgd.txt"), bPipe("s/pg.txt"),
    gmeOvr("s/go.txt");

    Button play("s/play.txt", 100, 20, &start);
    Button col("s/play.txt", 160, 20, &collision);
    Button replay("s/play.txt", 200, 100, &restart);

    int fF = 0;

    while (1) 
    {
        LCD.Update();
        LCD.Clear();
        bg.display(bgX, 0);
        bg.display(bgX + 132, 0);
        bg.display(bgX + 264, 0);

        bgX -= bgVel;
        fgX -= fgVel;
        fF++;
        fF %= 100;

        switch (status)
        {
            case 0:    // Start menu
                smooth += .04;
                y = (int) (sin(smooth) * 30) + 100;
                base.display(fgX, 195);
                yb[(int) (fF / 3.0) % 4].display(50, y);
                play.display(play.x, play.y);
                if (LCD.Touch(&xt, &yt))
                {
                    play.update();
                }
                break;

            case 1:    // Game in progress 
                score += 1;
                showScore((int) score, nums);

                y += yVel;
                yVel += g;
                tPipe.display(fgX, -240);
                bPipe.display(fgX, 150);
                base.display(fgX, 195);
                yb[(int) (fF / 3.0) % 4].display(50, y);
                col.display(col.x, col.y);
        
                if (LCD.Touch(&xt, &yt) || y > 240 - 24 - 5)
                {
                    col.update();
                    yVel = bounceVel;
                }
                break;
            
            case 2:    // Game over
                gmeOvr.display(20, 20);
                replay.display(replay.x, replay.y);
                if (LCD.Touch(&xt, &yt))
                {
                    replay.update();
                }
                break;
        }
    }
    return 0;
}


