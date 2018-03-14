#include <string.h>  // import memset

#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>

#include "modules/screen.h"
#include "modules/uploader.h"
#include "modules/reader.h"

int main() {
    {
        Screen   screen;
        Uploader uploader;
        Reader   reader;

        screen.init();
        screen.refresh(Status::FREE, reader, uploader);

        SceCtrlData input;
        memset(&input, 0, sizeof(input));

        while (true) {
            sceCtrlPeekBufferPositive(0, &input, 1);
            if (input.buttons & SCE_CTRL_SELECT) {
                // exit app
                break;
            } else if (input.buttons & SCE_CTRL_LTRIGGER) {
                screen.refresh(Status::SEARCHING, reader, uploader);
                reader.readLicences();
                sceKernelDelayThread(1000000);
                screen.refresh(Status::FREE, reader, uploader);
            } else if (input.buttons & SCE_CTRL_RTRIGGER) {
                screen.refresh(Status::UPLOADING, reader, uploader);
                uploader.upload(reader);
                sceKernelDelayThread(1000000);
                screen.refresh(Status::FREE, reader, uploader);
            }
            sceKernelDelayThread(1000);
        }

        screen.free();
    }

    sceKernelExitProcess(0);
    return 0;
}
