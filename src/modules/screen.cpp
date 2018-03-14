#include "modules/screen.h"

namespace {
constexpr const char* HOMEPAGE = "https://github.com/drinkmystery/vita2npsc";
}

int Screen::init() {
    vita2d_init();
    vita2d_set_clear_color(RGBA8(0, 0, 0, 0xFF));
    _pgf = vita2d_load_default_pgf();
    return 0;
}

int Screen::free() {
    vita2d_wait_rendering_done();
    vita2d_free_pgf(_pgf);
    vita2d_fini();
    return 0;
}

int Screen::refresh(Status status, const Reader& reader, const Uploader& uploader) {
    vita2d_start_drawing();
    vita2d_clear_screen();

    switch (status) {
        case Status::SEARCHING:
            vita2d_pgf_draw_text(_pgf, 200, 30, RGBA8(0, 255, 0, 255), 1.0f, "Searching fake license...");
            break;
        case Status::UPLOADING:
            vita2d_pgf_draw_text(
                _pgf, 200, 30, RGBA8(0, 255, 0, 255), 1.0f, "Uploading fake license to nopaystation database...");
            break;
        case Status::FREE:
        default:
            vita2d_pgf_draw_text(
                _pgf, 200, 30, RGBA8(255, 255, 255, 255), 1.0f, "press L search, press R upload, press select exit.");
            break;
    }

    vita2d_pgf_draw_textf(_pgf,
                          200,
                          100,
                          RGBA8(255, 255, 255, 255),
                          1.0f,
                          "Found %d game(s) and %d dlc(s) nonpdrm fake license(s).",
                          reader.games.size(),
                          reader.dlcs.size());

    vita2d_pgf_draw_textf(_pgf, 200, 150, RGBA8(255, 255, 255, 255), 1.0f, "%s", uploader.upload_result.c_str());
    vita2d_pgf_draw_textf(_pgf, 200, 200, RGBA8(255, 255, 255, 255), 1.0f, "%s", uploader.save_result.c_str());

    vita2d_pgf_draw_textf(_pgf, 200, 500, RGBA8(255, 255, 255, 255), 1.0f, "%s", HOMEPAGE);

    vita2d_end_drawing();
    vita2d_swap_buffers();

    return 0;
}
