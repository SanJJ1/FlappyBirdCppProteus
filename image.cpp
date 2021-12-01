#include "FEHLCD.h"
#include <stdio.h>

unsigned int *buffer;


// class img
// {
//     public:
//     private:
//         unsigned int *buffer;
// };

void readImgFile(int w, int h, char *fname) 
{
    FILE *f = fopen(fname, "rb");
    buffer = new unsigned int[w * h];
    fseek(f, 54, SEEK_SET);
    fread(buffer, 4, w * h, f);
    fclose(f);
}

unsigned int getRGBAtPxlInImg(int x, int y, int w, int h, int offset) 
{
    //assuming your x/y starts from top left, like I usually do
    return buffer[((h - 1) - y) * w + x + offset];
}

void drawImg(int x, int y, int w, int h, int offset, char *fname)
{
    readImgFile(w, h, fname);
    for (int i = 0; i < w; i++)
    {
        for (int j = 1; j < h - 1; j++)
        {
            unsigned int col = getRGBAtPxlInImg(i, j, w, h, offset); 
            if (col & 0xFF000000 && y + j < 240 && y + j > 0)
            {
                LCD.SetFontColor(col);
                LCD.DrawPixel(x + i, y + j);
            }
            // LCD.SetFontColor(col);
            // LCD.DrawPixel(x + i, y + j);
        }
    }
}


/*
 * Entry point to the application
 */
int main() 
{
    // Clear background
    LCD.SetBackgroundColor(LCD.Gray);
    LCD.Clear();
    LCD.SetFontColor(LCD.White);
    LCD.FillRectangle(20, 20, 20, 20);
    LCD.DrawPixel(50, 50);
    int x = 50;
    float y = 0;
    int xt, yt;
    float g = .7;
    float yVel = 0;
    float bgVel = 1;
    float bounceVel = -6;
    // char f[] = "sprites/logo4.bmp";
    char f2[] = "sprites/yellowbird-midflap.bmp";
    char bg[] = "sprites/background-day.bmp";
    char base[] = "sprites/base.bmp";
    char tPipe[] = "sprites/pipe-green-down.bmp";
    char bPipe[] = "sprites/pipe-green.bmp";

    // readImgFile(64, 64, array);


    // LCD.WriteLine("Hello World!");
    x += 1;
    while (1) 
    {
        LCD.Update();
        LCD.Clear();
        y += yVel;
        yVel += g;
        x -= bgVel;

        drawImg(x, -1, 135, 240, 21, bg);
        drawImg(x + 132, -1, 135, 240, 21, bg);
        drawImg(x + 264, -1, 135, 240, 21, bg);

        drawImg(2 * x, -240, 52, 320, 17, tPipe);
        drawImg(2 * x, 150, 52, 320, 17, bPipe); 

        drawImg(2 * x, 205, 320, 107, 17, base);
        drawImg(50, (int) y, 34, 24, 20, f2);

        if (LCD.Touch(&xt, &yt))
        {
            // while (LCD.Touch(&xt, &yt))
            // {

            // }
            yVel = bounceVel;
        }

        if (y > 240 - 24 - 5)
        {
            yVel = bounceVel;
        }
    }
    return 0;
}


