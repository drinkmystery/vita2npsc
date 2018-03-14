#ifndef _UPLOADER_H_
#define _UPLOADER_H_

#include <string>
#include <vector>

#include "modules/reader.h"

class Uploader {
public:
    Uploader();
    ~Uploader();
    int upload(const Reader& reader);

    std::string upload_result;
    std::string save_result;

private:
    int save(const std::string& result);

    std::vector<char> _net_mem;
};

#endif  // _UPLOADER_H_