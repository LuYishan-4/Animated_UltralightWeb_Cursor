#include "header/MouseUpload.hpp"

#include <X11/Xlib.h>

#include <iostream>


namespace UltralightWebCursorM
{


class KdeMouseUploader :
    public IMouseUploader
{

public:

    ~KdeMouseUploader() override
    {
        if(display_)
        {
            XCloseDisplay(display_);
            display_ = nullptr;
        }
    }


    bool initialize() override
    {
        display_ = XOpenDisplay(nullptr);

        if(!display_) return false;
        

        screen_ = DefaultScreen(display_);
        root_ = RootWindow(display_, screen_);


        return true;
    }


    bool poll(MousePoint& out) override
    {
        if(!display_)
            return false;

        Window root_return;
        Window child_return;

        int root_x = 0;
        int root_y = 0;
        int win_x = 0;
        int win_y = 0;

        unsigned int mask_return = 0;

        Bool ok = XQueryPointer(
            display_,
            root_,
            &root_return,
            &child_return,
            &root_x,
            &root_y,
            &win_x,
            &win_y,
            &mask_return
        );

        if(!ok)
            return false;

        out.x = root_x;
        out.y = root_y;
        out.pressed = (mask_return & Button1Mask) != 0;

        return true;
    }


private:

    Display* display_ = nullptr;
    int screen_ = 0;
    Window root_ = 0;

};


std::unique_ptr<IMouseUploader> createKdeMouseUploader()
{
    return std::make_unique<KdeMouseUploader>();
}


}
