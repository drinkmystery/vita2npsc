#include "modules/reader.h"

#include <stdio.h>
#include <string.h>  // import memset

#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>

extern "C" {
#include <PkgDecrypt/keyflate.h>
#include <PkgDecrypt/sfo.h>
#include <PkgDecrypt/rif.h>
#include <libb64/cencode.h>
}

namespace {
constexpr const char* GAMES_LIC_PATH = "ux0:nonpdrm/license/app";
constexpr const char* DLCS_LIC_PATH  = "ux0:nonpdrm/license/addcont";
constexpr const char* GAMES_PATH     = "ux0:app";
constexpr const char* DLCS_PATH      = "ux0:addcont";
}  // namespace

int Reader::readLicences() {
    games.clear();
    dlcs.clear();

    SceUID games_dir = sceIoDopen(GAMES_LIC_PATH);

    if (games_dir >= 0) {
        int left_title_count = 0;
        do {
            SceIoDirent entry_info = {0};

            left_title_count = sceIoDread(games_dir, &entry_info);

            if (left_title_count < 0) {
                break;
            }

            if (!SCE_S_ISDIR(entry_info.d_stat.st_mode)) {
                continue;
            }

            readGameLicense(entry_info.d_name);
        } while (left_title_count > 0);

        sceIoDclose(games_dir);
    }

    SceUID dlcs_dir = sceIoDopen(DLCS_LIC_PATH);

    if (dlcs_dir >= 0) {
        int left_title_count = 0;
        do {
            SceIoDirent entry_info = {0};

            left_title_count = sceIoDread(dlcs_dir, &entry_info);

            if (left_title_count < 0) {
                break;
            }

            if (!SCE_S_ISDIR(entry_info.d_stat.st_mode)) {
                continue;
            }

            readDlcsLicense(entry_info.d_name);
        } while (left_title_count > 0);

        sceIoDclose(dlcs_dir);
    }

    return 0;
}

int Reader::readGameLicense(const char* title_id) {
    std::string title_path = std::string(GAMES_LIC_PATH) + "/" + title_id;

    SceUID title_dir = sceIoDopen(title_path.c_str());
    if (title_dir < 0) {
        return -1;
    }

    int left_rif_count = 0;
    do {
        SceIoDirent entry_info = {0};

        left_rif_count = sceIoDread(title_dir, &entry_info);

        if (left_rif_count < 0) {
            break;
        }

        if (!SCE_S_ISREG(entry_info.d_stat.st_mode)) {
            continue;
        }

        Record      result;
        std::string rif_path = title_path + "/" + entry_info.d_name;

        if (parseRif(rif_path, result.content_id, result.zrif) != 0) {
            continue;
        }

        std::string sfo_content_id;
        std::string sfo_path = std::string(GAMES_PATH) + "/" + title_id + "/sce_sys/param.sfo";

        if (parseSfo(sfo_path, result.title_id, result.title, sfo_content_id) == 0) {
            if (result.title_id.compare(title_id) != 0 || result.content_id.compare(sfo_content_id) != 0) {
                continue;
            }
        }

        result.title_id = title_id;
        getRegion(result.title_id, result.region);

        games.push_back(result);
    } while (left_rif_count > 0);

    sceIoDclose(title_dir);

    return 0;
}

int Reader::readDlcLicense(const char* title_id, const char* dlc_folder) {
    std::string dlc_path = std::string(DLCS_LIC_PATH) + "/" + title_id + "/" + dlc_folder;

    SceUID dlc_dir = sceIoDopen(dlc_path.c_str());
    if (dlc_dir < 0) {
        return -1;
    }

    int left_rif_count = 0;
    do {
        SceIoDirent entry_info = {0};

        left_rif_count = sceIoDread(dlc_dir, &entry_info);

        if (left_rif_count < 0) {
            break;
        }

        if (!SCE_S_ISREG(entry_info.d_stat.st_mode)) {
            continue;
        }

        Record      result;
        std::string rif_path = dlc_path + "/" + entry_info.d_name;

        if (parseRif(rif_path, result.content_id, result.zrif) != 0) {
            continue;
        }

        std::string sfo_content_id;
        std::string sfo_path = std::string(DLCS_PATH) + "/" + title_id + "/" + dlc_folder + "/sce_sys/param.sfo";

        if (parseSfo(sfo_path, result.title_id, result.title, sfo_content_id) == 0) {
            if (result.title_id.compare(title_id) != 0 || result.content_id.compare(sfo_content_id) != 0) {
                continue;
            }
        }

        result.title_id = title_id;
        getRegion(result.title_id, result.region);

        dlcs.push_back(result);
    } while (left_rif_count > 0);

    sceIoDclose(dlc_dir);

    return 0;
}

int Reader::readDlcsLicense(const char* title_id) {
    std::string dlcs_path = std::string(DLCS_LIC_PATH) + "/" + title_id;

    SceUID dlcs_dir = sceIoDopen(dlcs_path.c_str());
    if (dlcs_dir < 0) {
        return -1;
    }

    int left_dlc_count = 0;
    do {
        SceIoDirent entry_info = {0};

        left_dlc_count = sceIoDread(dlcs_dir, &entry_info);

        if (left_dlc_count < 0) {
            break;
        }

        if (!SCE_S_ISDIR(entry_info.d_stat.st_mode)) {
            continue;
        }

        readDlcLicense(title_id, entry_info.d_name);
    } while (left_dlc_count > 0);

    sceIoDclose(dlcs_dir);

    return 0;
}

int Reader::parseRif(const std::string& path, std::string& content_id, std::string& zrif) {
    int result = 0;

    SceIoStat stat = {0};

    if (sceIoGetstat(path.c_str(), &stat) < 0) {
        return -1;
    }

    if (!SCE_S_ISREG(stat.st_mode)) {
        return -2;
    }

    FILE* lic = fopen(path.c_str(), "rb");

    if (lic) {
        char key[MAX_KEY_SIZE];
        int  len = fread(key, 1, MAX_KEY_SIZE, lic);

        if (len < MIN_KEY_SIZE) {
            result = -3;
        } else {
            if (*((uint16_t*)(key + 4)) != 0) {
                SceNpDrmLicense* license = (SceNpDrmLicense*)key;

                // Check if it is a NoNpDRM license
                if (license->aid != FAKE_AID) {
                    license->aid = FAKE_AID;
                }

                content_id += license->content_id;
            } else {
                ScePsmDrmLicense* license = (ScePsmDrmLicense*)key;

                if (license->aid != FAKE_AID) {
                    license->aid = FAKE_AID;
                }

                content_id += license->content_id;
            }

            unsigned char out[MAX_KEY_SIZE];
            memset(out, 0, MAX_KEY_SIZE);

            if ((len = deflateKey((unsigned char*)key, len, out, MAX_KEY_SIZE)) >= 0) {
                // Align len to 3 byte block to avoid padding by base64
                if ((len % 3) > 0)
                    len += 3 - (len % 3);

                // Everything was ok, now encode binary buffer into base64 string and print in the stdout
                memset(key, 0, MAX_KEY_SIZE);
                base64_encodestate state;
                base64_init_encodestate(&state);
                int enc_len = base64_encode_block((char*)out, len, key, &state);
                enc_len += base64_encode_blockend(key + enc_len, &state);

                zrif += key;
                zrif.resize(zrif.length() - 1);
            }
        }

        fclose(lic);
    } else {
        result = -4;
    }

    return result;
}

int Reader::parseSfo(const std::string& path, std::string& title_id, std::string& title, std::string& content_id) {
    SceIoStat stat = {0};

    if (sceIoGetstat(path.c_str(), &stat) < 0) {
        return -1;
    }

    if (!SCE_S_ISREG(stat.st_mode)) {
        return -2;
    }

    PSF sfo = psfRead(path.c_str());

    if (sfo) {
        title_id   = psfGetString(sfo, "TITLE_ID");
        title      = psfGetString(sfo, "TITLE");
        content_id = psfGetString(sfo, "CONTENT_ID");

        psfDiscard(sfo);

        return 0;
    }

    return -3;
}

int Reader::getRegion(const std::string& title_id, std::string& region) {
    if (title_id.compare(0, 4, "PCSE") == 0 || title_id.compare(0, 4, "PCSA") == 0 ||
        title_id.compare(0, 4, "NPNA") == 0) {
        region = "US";
    } else if (title_id.compare(0, 4, "PCSF") == 0 || title_id.compare(0, 4, "PCSB") == 0 ||
               title_id.compare(0, 4, "NPOA") == 0) {
        region = "EU";
    } else if (title_id.compare(0, 4, "PCSC") == 0 || title_id.compare(0, 4, "VCJS") == 0 ||
               title_id.compare(0, 4, "PCSG") == 0 || title_id.compare(0, 4, "VLJS") == 0 ||
               title_id.compare(0, 4, "VLJM") == 0 || title_id.compare(0, 4, "NPPA") == 0) {
        region = "JP";
    } else if (title_id.compare(0, 4, "VCAS") == 0 || title_id.compare(0, 4, "PCSH") == 0 ||
               title_id.compare(0, 4, "VLAS") == 0 || title_id.compare(0, 4, "PCSD") == 0 ||
               title_id.compare(0, 4, "NPQA") == 0) {
        region = "ASIA";
    } else {
        region = "unknown region";
    }

    return 0;
}

// int Reader::debug(const std::string &msg)
// {
//     SceUID fd = sceIoOpen("ux0:data/vita2nps_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
//     if (fd >= 0)
//     {
//         sceIoWrite(fd, msg.c_str(), msg.length());
//         sceIoWrite(fd, "\n", 1);
//         sceIoClose(fd);
//     }
// }
