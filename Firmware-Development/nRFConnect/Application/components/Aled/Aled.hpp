#ifndef _ALED_RGB_H_
#define _ALED_RGB_H_

#define ALED_RGB_DELAY_TIME K_SECONDS(1)

enum aled_rgb_e
{
    BLACK,
    RED,
    GREEN,
    BLUE,
    WHITE

};

class ALED
{
public:
    void init();
    void setColor(int color, int pixel_index);
};

#endif // _ALED_RGB_H_