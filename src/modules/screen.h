#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <vita2d.h>

#include "modules/reader.h"
#include "modules/uploader.h"

enum Status { FREE, SEARCHING, UPLOADING };

class Screen {
public:
    Screen()  = default;
    ~Screen() = default;
    int init();
    int free();
    int refresh(Status status, const Reader& reader, const Uploader& uploader);

private:
    vita2d_pgf* _pgf = nullptr;
};

#endif  // _SCREEN_H_