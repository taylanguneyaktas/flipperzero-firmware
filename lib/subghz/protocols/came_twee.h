#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_CAME_TWEE_NAME "CAME TWEE"

typedef struct SubGhzProtocolDecoderCameTwee SubGhzProtocolDecoderCameTwee;
typedef struct SubGhzProtocolEncoderCameTwee SubGhzProtocolEncoderCameTwee;

extern const SubGhzProtocolDecoder subghz_protocol_came_twee_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_came_twee_encoder;
extern const SubGhzProtocol subghz_protocol_came_twee;

void* subghz_protocol_encoder_came_twee_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_came_twee_free(void* context);
bool subghz_protocol_encoder_came_twee_load(
    void* context,
    uint64_t key,
    uint8_t count_bit,
    size_t repeat);
void subghz_protocol_encoder_came_twee_stop(void* context);
LevelDuration subghz_protocol_encoder_came_twee_yield(void* context);
void* subghz_protocol_decoder_came_twee_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_came_twee_free(void* context);
void subghz_protocol_decoder_came_twee_reset(void* context);
void subghz_protocol_decoder_came_twee_feed(void* context, bool level, uint32_t duration);
void subghz_protocol_decoder_came_twee_serialization(void* context, string_t output);
bool subghz_protocol_came_twee_save_file(void* context, FlipperFile* flipper_file);
bool subghz_protocol_came_twee_load_file(
    void* context,
    FlipperFile* flipper_file,
    const char* file_path);