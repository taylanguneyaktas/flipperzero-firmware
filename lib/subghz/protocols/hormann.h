#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HORMANN_HSM_NAME "Hormann HSM"

typedef struct SubGhzProtocolDecoderHormann SubGhzProtocolDecoderHormann;
typedef struct SubGhzProtocolEncoderHormann SubGhzProtocolEncoderHormann;

extern const SubGhzProtocolDecoder subghz_protocol_hormann_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_hormann_encoder;
extern const SubGhzProtocol subghz_protocol_hormann;

void* subghz_protocol_encoder_hormann_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_hormann_free(void* context);
bool subghz_protocol_encoder_hormann_load(
    void* context,
    uint64_t key,
    uint8_t count_bit,
    size_t repeat);
void subghz_protocol_encoder_hormann_stop(void* context);
LevelDuration subghz_protocol_encoder_hormann_yield(void* context);
void* subghz_protocol_decoder_hormann_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_hormann_free(void* context);
void subghz_protocol_decoder_hormann_reset(void* context);
void subghz_protocol_decoder_hormann_feed(void* context, bool level, uint32_t duration);
void subghz_protocol_decoder_hormann_serialization(void* context, string_t output);
bool subghz_protocol_hormann_save_file(void* context, FlipperFile* flipper_file);
bool subghz_protocol_hormann_load_file(
    void* context,
    FlipperFile* flipper_file,
    const char* file_path);