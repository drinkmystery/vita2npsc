#include "modules/uploader.h"

#include <psp2/sysmodule.h>
#include <psp2/io/fcntl.h>
#include <psp2/net/net.h>

#include <restclient-cpp/restclient.h>

namespace {
constexpr const char* SERVER = "http://us-central1-vita2nps.cloudfunctions.net/forwarder/add";
}

Uploader::Uploader() {
    _net_mem.reserve(1024 * 1024);
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    SceNetInitParam param;
    param.memory = &(_net_mem[0]);
    param.size   = 1024 * 1024;
    param.flags  = 0;
    sceNetInit(&(param));
}

Uploader::~Uploader() {
    sceNetTerm();
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

int Uploader::upload(const Reader& reader) {
    upload_result = "";
    save_result   = "";

    if (reader.games.empty() && reader.dlcs.empty()) {
        upload_result = "Nothing to upload!!";
        return 0;
    }

    std::string buffer;
    buffer.reserve((reader.games.size() + reader.dlcs.size()) * 512);

    buffer.append("Platform");
    buffer.append(",");
    buffer.append("Title ID");
    buffer.append(",");
    buffer.append("Region");
    buffer.append(",");
    buffer.append("Name");
    buffer.append(",");
    buffer.append("PKG direct link");
    buffer.append(",");
    buffer.append("zRIF");
    buffer.append(",");
    buffer.append("Content ID");
    buffer.append(",");
    buffer.append("Original Name");
    buffer.append("\n");

    for (auto& game : reader.games) {
        buffer.append("PSV");
        buffer.append(",");
        buffer.append(game.title_id);
        buffer.append(",");
        buffer.append(game.region);
        buffer.append(",");
        buffer.append(game.title);
        buffer.append(",");
        // pkg link
        buffer.append(",");
        buffer.append(game.zrif);
        buffer.append(",");
        buffer.append(game.content_id);
        buffer.append(",");
        buffer.append(game.title);
        buffer.append("\n");
    }

    for (auto& dlc : reader.dlcs) {
        buffer.append("PSV DLC");
        buffer.append(",");
        buffer.append(dlc.title_id);
        buffer.append(",");
        buffer.append(dlc.region);
        buffer.append(",");
        buffer.append(dlc.title);
        buffer.append(",");
        // pkg link
        buffer.append(",");
        buffer.append(dlc.zrif);
        buffer.append(",");
        buffer.append(dlc.content_id);
        buffer.append(",");
        buffer.append(dlc.title);
        buffer.append("\n");
    }

    RestClient::Response r = RestClient::post(SERVER, "text/plain", buffer);

    upload_result = r.body;

    save(buffer);

    return 0;
}

int Uploader::save(const std::string& result) {
    SceUID fd = sceIoOpen("ux0:data/vita2nps.csv", SCE_O_WRONLY | SCE_O_CREAT, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, result.c_str(), result.length());
        sceIoClose(fd);
        save_result = "Csv file also save to ux0:data/vita2nps.csv.";
    }
    return 0;
}
