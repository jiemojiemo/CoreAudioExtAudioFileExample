
//
// Created by William.Hua on 2020/11/22.
//
#include "tool_function.h"
#include <AudioToolbox/ExtendedAudioFile.h>
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[])
{
    if(argc < 2){
        cerr << "Usage: decode /full/path/to/audiofile\n";
        return -1;
    }

    CFURLRef audio_url = createCFURLWithStdString(string(argv[1]));
    ON_SCOPE_EXIT([audio_url](){CFRelease(audio_url);});

    ExtAudioFileRef file;
    auto status = ExtAudioFileOpenURL(audio_url, &file);
    assert(status == noErr);

    SInt64 file_frame_length;
    UInt32 size = sizeof(file_frame_length);
    status = ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileLengthFrames,
                                     &size, &file_frame_length);
    assert(status == noErr);
    cout << "file frame length:" << file_frame_length << endl;

    // get same basic information about input file
    AudioStreamBasicDescription input_asbd;
    size = sizeof(input_asbd);
    ExtAudioFileGetProperty(file, kExtAudioFileProperty_FileDataFormat,
                            &size, &input_asbd);

    UInt32 format4cc = CFSwapInt32HostToBig(input_asbd.mFormatID);
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

    return 0;
}
