//
// Created by bytedance on 2021/12/2.
//

#include "file_output_stream.h"
#include <iostream>
FileOutputStream::FileOutputStream(const std::string& path)
{
    open(path);
}

FileOutputStream::~FileOutputStream()
{
    close();
}

bool FileOutputStream::isOpen() const
{
    return fp_ != nullptr;
}

int FileOutputStream::open(const std::string& path)
{
    fp_ = fopen(path.c_str(), "wb+");
    if(!fp_){
        return -1;
    }
    return 0;
}

void FileOutputStream::close()
{
    if(fp_)
    {
        fclose(fp_);
        fp_ = nullptr;
    }
}

int64_t FileOutputStream::write(const uint8_t* source_buf, int64_t size)
{
    if(!isOpen()){
        return -1;
    }

    return fwrite(source_buf, sizeof(uint8_t), size, fp_);
}

int64_t FileOutputStream::read(uint8_t* dest_buf, int64_t size)
{
    if(!isOpen()){
        return -1;
    }
    auto ret = fread(dest_buf, 1, size, fp_);
    if(ret != size){
        if(ferror(fp_))
        {
            std::cerr << "error reading" << std::endl;
            return -1;
        }
    }
    return ret;
}


int64_t FileOutputStream::tellp() const
{
    if(!isOpen()){
        return 0;
    }

    return ftell(fp_);
}

int FileOutputStream::seekp(int64_t pos, int mode)
{
    if(!isOpen()){
        return -1;
    }
    return fseek(fp_, pos, mode);
}

int64_t FileOutputStream::length()
{
    if(!isOpen()){
        return 0;
    }else
    {
        auto cur_pos = tellp();
        seekp(0, SEEK_END);
        auto length = tellp();
        seekp(cur_pos, SEEK_SET);

        return length;
    }

}