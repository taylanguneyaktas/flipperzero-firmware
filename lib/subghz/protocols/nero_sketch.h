#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NERO_SKETCH_NAME "Nero Sketch"

typedef struct SubGhzProtocolDecoderNeroSketch SubGhzProtocolDecoderNeroSketch;
typedef struct SubGhzProtocolEncoderNeroSketch SubGhzProtocolEncoderNeroSketch;

extern const SubGhzProtocolDecoder subghz_protocol_nero_sketch_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nero_sketch_encoder;
extern const SubGhzProtocol subghz_protocol_nero_sketch;

void* subghz_protocol_encoder_nero_sketch_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_nero_sketch_free(void* context);
bool subghz_protocol_encoder_nero_sketch_load(
    void* context,
    uint64_t key,
    uint8_t count_bit,
    size_t repeat);
void subghz_protocol_encoder_nero_sketch_stop(void* context);
LevelDuration subghz_protocol_encoder_nero_sketch_yield(void* context);
void* subghz_protocol_decoder_nero_sketch_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_nero_sketch_free(void* context);
void subghz_protocol_decoder_nero_sketch_reset(void* context);
void subghz_protocol_decoder_nero_sketch_feed(void* context, bool level, uint32_t duration);
void subghz_protocol_decoder_nero_sketch_serialization(void* context, string_t output);
bool subghz_protocol_nero_sketch_save_file(void* context, FlipperFile* flipper_file);
bool subghz_protocol_nero_sketch_load_file(
    void* context,
    FlipperFile* flipper_file,
    const char* file_path);