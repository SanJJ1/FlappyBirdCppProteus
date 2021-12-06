#include "FEHLCD.h"
#include "FEHSD.h"
#include "FEHUtility.h"
#include <math.h>
// #include <iostream>
// using namespace std;

class Image
{
    public:
        Image(const char *);
        void display(int, int);
    private:
        unsigned int *data;
        const char *fn;
        int w, h;
};


Image::Image(const char *fname)
{
    fn = fname;
    FEHFile *fptr = SD.FOpen(fname, "r");
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

void Image::display(int x, int y)
{
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            unsigned int col = data[j * w + i];
            if (col & 0xFF000000 && y + j < 240 && y + j >= 0)
            {
                LCD.SetFontColor(0xFFFFFFFF - col);
                LCD.DrawPixel(x + i, y + j);
            }
        }
    }
}

int main() 
{
    LCD.SetBackgroundColor(LCD.Blue);     // Clear background
    LCD.Clear();

    // background and foreground positions and velocities
    float bgX = 50, bgVel = 1, fgX = 50, fgVel = 2;

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


    int fF = 0;
    while (1) 
    {
        LCD.Update();
        LCD.Clear();
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

        base.display(fgX, 195);

        yb[(int) (fF / 3.0) % 4].display(50, (int) (sin(smooth) * 30) + 100);
        yb[(int) (fF / 3.0) % 4].display(50, y);
        
 
        if (LCD.Touch(&xt, &yt) || y > 240 - 24 - 5)
        {
            yVel = bounceVel;
        }
    }
    return 0;
}


