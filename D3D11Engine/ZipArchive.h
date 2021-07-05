#pragma once
#ifdef _ADD_ZIPARCHIVE_
#include "pch.h"
#include <thread>

// zipname, userdata, unpackedfiles, numfiles
typedef void( __cdecl* UnzipDoneCallback )(const std::string&, void*, int, int);

class ZipArchive {
public:
    ZipArchive();
    ~ZipArchive();

    /** unzips the given archive */
    static XRESULT Unzip( const std::string& zip, const std::string& target );
    XRESULT UnzipThreaded( const std::string& zip, const std::string& target, UnzipDoneCallback callback, void* cbUserdata );

private:

    static void UnzipThreadFunc( const std::string& zip, const std::string& target, UnzipDoneCallback callback, void* cbUserdata );

    std::thread* UnzipThread;
};

#endif