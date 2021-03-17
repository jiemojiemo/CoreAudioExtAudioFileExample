
//
// Created by William.Hua on 2021/03/17.
//

#include "tool_function.h"
#include <iostream>
#include <fstream>
#include <AudioToolbox/AudioToolbox.h>

using namespace std;

class Reader
{
public:
    int open(const std::string& file_path){
        in_file.open(file_path);

        if(!in_file.is_open()){
            file_size = 0;
            return -1;
        }

        // get file size
        initFileSize();

        return 0;
    }

    SInt64 getFileSize() const{
        return file_size;
    }

    static OSStatus readCallback(
        void *		inClientData,
        SInt64		inPosition,
        UInt32		requestCount,
        void *		buffer,
        UInt32 *	actualCount)
    {
        auto* reader = static_cast<Reader*>(inClientData);

        if(!reader->in_file.is_open()){
            cerr << "cannot read before open file" << endl;
            return -1;
        }

        reader->in_file.seekg(inPosition, std::ios::beg);
        *actualCount = reader->in_file.read((char*)buffer, requestCount).gcount();
        return noErr;
    }

    static SInt64 getSizeCallback(void* inClientData){
        return static_cast<Reader*>(inClientData)->getFileSize();
    }

    fstream in_file;
    SInt64 file_size;
private:
    void initFileSize(){
        in_file.seekg(0, in_file.end);
        file_size = in_file.tellg();
        in_file.seekg(0, in_file.beg);
    }
};


int main(int argc, char* argv[])
{
    if(argc < 2){
        cerr << "Usage: decode /full/path/to/audiofile\n";
        return -1;
    }

    Reader reader;
    reader.open(argv[1]);

    AudioFileID audio_file_id;
    OSStatus status = AudioFileOpenWithCallbacks(&reader,
                                                 &Reader::readCallback,
                                                 nullptr,
                                                 &Reader::getSizeCallback,
                                                 nullptr,
                                                 0,
                                                 &audio_file_id);
    assert(status == noErr);

    UInt32 file_type;
    UInt32 size = sizeof(file_type);
    AudioFileGetProperty(audio_file_id, kAudioFilePropertyFileFormat, &size, &file_type);
    AudioFileClose(audio_file_id);

    // open audio file with format hint
    CFURLRef audio_url = createCFURLWithStdString(string(argv[1]));
    ON_SCOPE_EXIT([audio_url](){CFRelease(audio_url);});
    status = AudioFileOpenURL(audio_url,kAudioFileReadPermission, file_type, &audio_file_id);
    assert(status == noErr);


    // open ext audio file
    ExtAudioFileRef file;
    status = ExtAudioFileWrapAudioFileID(audio_file_id, false, &file);
    assert(status == noErr);

    SInt64 file_frame_length;
    size = sizeof(file_frame_length);
    status = ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileLengthFrames,
                                     &size, &file_frame_length);
    assert(status == noErr);
    cout << "file frame length:" << file_frame_length << endl;

    // get same basic information about input file
    AudioStreamBasicDescription input_asbd;
    size = sizeof(input_asbd);
    ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileDataFormat,
                            &size, &input_asbd);

    auto format4cc = CFSwapInt32HostToBig(input_asbd.mFormatID);
    cout << "file format:" << (char*)(&format4cc) << endl;
    cout << "is compressed: " << ((input_asbd.mBitsPerChannel == 0) ? "YES":"NO") << endl;
    cout << "sample rate:" << input_asbd.mSampleRate << endl;
    cout << "number of channels:" << input_asbd.mChannelsPerFrame << endl;

    // set output file format
    AudioStreamBasicDescription output_asbd;
    FillOutASBDForLPCM(output_asbd, 44100, 2, 32, 32, true, false, false);

    size = sizeof(output_asbd);
    status = ExtAudioFileSetProperty(file, kExtAudioFileProperty_ClientDataFormat,
                                     size,&output_asbd);

    assert(status == noErr);

    // now, we can read pcm from file
    SInt64 frame_offset = 0;
    UInt32 num_frame_read = 1024;
    AudioBufferList buffer_list;
    buffer_list.mNumberBuffers = 1;
    buffer_list.mBuffers[0].mNumberChannels = output_asbd.mChannelsPerFrame;
    buffer_list.mBuffers[0].mDataByteSize = num_frame_read * output_asbd.mBytesPerFrame;
    buffer_list.mBuffers[0].mData = malloc(buffer_list.mBuffers[0].mDataByteSize);

    fstream out("out.raw", std::ios::out | std::ios::binary);
    for(;frame_offset < file_frame_length;){
        status = ExtAudioFileRead(file, &num_frame_read, &buffer_list);
        assert(status == noErr);

        const auto num_read_bytes = num_frame_read * output_asbd.mBytesPerFrame;
        out.write((char*)(buffer_list.mBuffers[0].mData), num_read_bytes);

        frame_offset += num_frame_read;
    }

    free(buffer_list.mBuffers[0].mData);
    ExtAudioFileDispose(file);
    AudioFileClose(audio_file_id);

    return 0;
}