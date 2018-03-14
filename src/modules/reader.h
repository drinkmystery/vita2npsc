#ifndef _READER_H_
#define _READER_H_

#include <string>
#include <vector>

struct Record {
    std::string title_id;
    std::string region;
    std::string title;
    std::string zrif;
    std::string content_id;
};

class Reader {
public:
    Reader()  = default;
    ~Reader() = default;
    int readLicences();

    std::vector<Record> games;
    std::vector<Record> dlcs;

private:
    int readGameLicense(const char* title_id);
    int readDlcLicense(const char* title_id, const char* dlc_folder);
    int readDlcsLicense(const char* title_id);
    int parseRif(const std::string& path, std::string& content_id, std::string& zrif);
    int parseSfo(const std::string& path, std::string& title_id, std::string& title, std::string& content_id);
    int getRegion(const std::string& title_id, std::string& region);
    // int debug(const std::string &msg);
};

#endif  // _READER_H_