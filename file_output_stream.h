//
// Created by bytedance on 2021/12/2.
//

#ifndef COREAUDIOEXTAUDIOFILEEXAMPLE_FILE_OUTPUT_STREAM_H
#define COREAUDIOEXTAUDIOFILEEXAMPLE_FILE_OUTPUT_STREAM_H
#include <string>
class OutputStream
{
public:
    virtual ~OutputStream() = default;

    /**
     * write to file
     * @param source_buf the source of data
     * @param size number of size to write
     * @return number of written bytes, returns -1 if failed
     */
    virtual int64_t write(const uint8_t* source_buf, int64_t size) = 0;

    /**
     * read from file
     * @param dest_buf the destination buffer
     * @param size size of bytes
     * @return number of read bytes, return -1 if failed
     */
    virtual int64_t read(uint8_t* dest_buf, int64_t size) = 0;

    /**
     * returns the write pointer position
     */
    virtual int64_t tellp() const = 0;

    /**
     * seek the write pointer
     * @param pos the new pos
     * @param mode SEEK_SET	Beginning of file
                   SEEK_CUR	Current position of the file pointer
                   SEEK_END End of file
     * @return 0 if seek successfully, others failed
     */
    virtual int seekp(int64_t pos, int mode) = 0;

    /**
     * return the length of output stream
     */
    virtual int64_t length() = 0;
};

class FileOutputStream : public OutputStream
{
public:
    FileOutputStream() = default;
    explicit FileOutputStream(const std::string& path);

    ~FileOutputStream() override;

    bool isOpen() const;

    int open(const std::string& path);

    void close();

    int64_t write(const uint8_t* source_buf, int64_t size) override;

    int64_t read(uint8_t* dest_buf, int64_t size) override;

    int64_t tellp() const override;

    int seekp(int64_t pos, int mode) override;

    int64_t length() override;

private:
    FILE* fp_{nullptr};
};


#endif //COREAUDIOEXTAUDIOFILEEXAMPLE_FILE_OUTPUT_STREAM_H
