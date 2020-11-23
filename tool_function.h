
//
// Created by William.Hua on 2020/9/28.
//

#ifndef LEARN_CORE_AUDIO_INCLUDE_TOOL_FUNCTION_H
#define LEARN_CORE_AUDIO_INCLUDE_TOOL_FUNCTION_H

#include "scope_guard.h"
#include <AudioToolbox/AudioToolbox.h>
#include <string>

CFStringRef createCFStringUTF8WithStdString(const std::string& str)
{
    CFStringRef cf_str = CFStringCreateWithCString(kCFAllocatorDefault,
                                                          str.c_str(),
                                                          kCFStringEncodingUTF8);
    return cf_str;
}

CFURLRef createCFURLWithStdString(const std::string& str)
{
    CFStringRef cf_str = createCFStringUTF8WithStdString(str);

    CFURLRef urf_ref = CFURLCreateWithString(kCFAllocatorDefault, cf_str, NULL);
    CFRelease(cf_str);

    return urf_ref;
}

#endif //LEARN_CORE_AUDIO_INCLUDE_TOOL_FUNCTION_H
