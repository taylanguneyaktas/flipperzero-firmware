#include "nero_sketch.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * Help
 * https://phreakerclub.com/447
 *
 */

#define TAG "SubGhzProtocolNeroSketch"

static const SubGhzBlockConst subghz_protocol_nero_sketch_const = {
    .te_short = 330,
    .te_long = 660,
    .te_delta = 150,
    .min_count_bit_for_found = 40,
};

struct SubGhzProtocolDecoderNeroSketch {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
    uint16_t header_count;
};

struct SubGhzProtocolEncoderNeroSketch {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    NeroSketchDecoderStepReset = 0,
    NeroSketchDecoderStepCheckPreambula,
    NeroSketchDecoderStepSaveDuration,
    NeroSketchDecoderStepCheckDuration,
} NeroSketchDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_nero_sketch_decoder = {
    .alloc = subghz_protocol_decoder_nero_sketch_alloc,
    .free = subghz_protocol_decoder_nero_sketch_free,

    .feed = subghz_protocol_decoder_nero_sketch_feed,
    .reset = subghz_protocol_decoder_nero_sketch_reset,

    .serialize = subghz_protocol_decoder_nero_sketch_serialization,
    .save_file = subghz_protocol_nero_sketch_save_file,
};

const SubGhzProtocolEncoder subghz_protocol_nero_sketch_encoder = {
    .alloc = subghz_protocol_encoder_nero_sketch_alloc,
    .free = subghz_protocol_encoder_nero_sketch_free,

    .load = subghz_protocol_encoder_nero_sketch_load,
    .stop = subghz_protocol_encoder_nero_sketch_stop,
    .yield = subghz_protocol_encoder_nero_sketch_yield,
    .load_file = subghz_protocol_nero_sketch_load_file,
};

const SubGhzProtocol subghz_protocol_nero_sketch = {
    .name = SUBGHZ_PROTOCOL_NERO_SKETCH_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_nero_sketch_decoder,
    .encoder = &subghz_protocol_nero_sketch_encoder,
};

void* subghz_protocol_encoder_nero_sketch_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolEncoderNeroSketch* instance =
        furi_alloc(sizeof(SubGhzProtocolEncoderNeroSketch));

    instance->base.protocol = &subghz_protocol_nero_sketch;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 256;
    instance->encoder.upload = furi_alloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_runing = false;
    return instance;
}

void subghz_protocol_encoder_nero_sketch_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderNeroSketch* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static bool subghz_protocol_came_encoder_get_upload(SubGhzProtocolEncoderNeroSketch* instance) {
    furi_assert(instance);

    size_t index = 0;
    size_t size_upload = 47 * 2 + 2 + (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header
    for(uint8_t i = 0; i < 47; i++) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nero_sketch_const.te_short);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_nero_sketch_const.te_short);
    }

    //Send start bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_nero_sketch_const.te_short * 4);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_nero_sketch_const.te_short);

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nero_sketch_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nero_sketch_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nero_sketch_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nero_sketch_const.te_long);
        }
    }

    //Send stop bit
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_nero_sketch_const.te_short * 3);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_nero_sketch_const.te_short);

    return true;
}

bool subghz_protocol_encoder_nero_sketch_load(
    void* context,
    uint64_t key,
    uint8_t count_bit,
    size_t repeat) {
    furi_assert(context);
    SubGhzProtocolEncoderNeroSketch* instance = context;
    instance->generic.data = key;
    instance->generic.data_count_bit = 40;
    instance->encoder.repeat = repeat;
    subghz_protocol_came_encoder_get_upload(instance);
    instance->encoder.is_runing = true;
    return true;
}

void subghz_protocol_encoder_nero_sketch_stop(void* context) {
    SubGhzProtocolEncoderNeroSketch* instance = context;
    instance->encoder.is_runing = false;
}

LevelDuration subghz_protocol_encoder_nero_sketch_yield(void* context) {
    SubGhzProtocolEncoderNeroSketch* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_runing) {
        instance->encoder.is_runing = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_nero_sketch_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderNeroSketch* instance =
        furi_alloc(sizeof(SubGhzProtocolDecoderNeroSketch));
    instance->base.protocol = &subghz_protocol_nero_sketch;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_nero_sketch_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroSketch* instance = context;
    free(instance);
}

void subghz_protocol_decoder_nero_sketch_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroSketch* instance = context;
    instance->decoder.parser_step = NeroSketchDecoderStepReset;
}

void subghz_protocol_decoder_nero_sketch_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroSketch* instance = context;

    switch(instance->decoder.parser_step) {
    case NeroSketchDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_nero_sketch_const.te_short) <
                       subghz_protocol_nero_sketch_const.te_delta)) {
            instance->decoder.parser_step = NeroSketchDecoderStepCheckPreambula;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;
    case NeroSketchDecoderStepCheckPreambula:
        if(level) {
            if((DURATION_DIFF(duration, subghz_protocol_nero_sketch_const.te_short) <
                subghz_protocol_nero_sketch_const.te_delta) ||
               (DURATION_DIFF(duration, subghz_protocol_nero_sketch_const.te_short * 4) <
                subghz_protocol_nero_sketch_const.te_delta)) {
                instance->decoder.te_last = duration;
            } else {
                instance->decoder.parser_step = NeroSketchDecoderStepReset;
            }
        } else if(
            DURATION_DIFF(duration, subghz_protocol_nero_sketch_const.te_short) <
            subghz_protocol_nero_sketch_const.te_delta) {
            if(DURATION_DIFF(
                   instance->decoder.te_last, subghz_protocol_nero_sketch_const.te_short) <
               subghz_protocol_nero_sketch_const.te_delta) {
                // Found header
                instance->header_count++;
                break;
            } else if(
                DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_nero_sketch_const.te_short * 4) <
                subghz_protocol_nero_sketch_const.te_delta) {
                // Found start bit
                if(instance->header_count > 40) {
                    instance->decoder.parser_step = NeroSketchDecoderStepSaveDuration;
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                } else {
                    instance->decoder.parser_step = NeroSketchDecoderStepReset;
                }
            } else {
                instance->decoder.parser_step = NeroSketchDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = NeroSketchDecoderStepReset;
        }
        break;
    case NeroSketchDecoderStepSaveDuration:
        if(level) {
            if(duration >= (subghz_protocol_nero_sketch_const.te_short * 2 +
                            subghz_protocol_nero_sketch_const.te_delta * 2)) {
                //Found stop bit
                instance->decoder.parser_step = NeroSketchDecoderStepReset;
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_nero_sketch_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                break;
            } else {
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = NeroSketchDecoderStepCheckDuration;
            }

        } else {
            instance->decoder.parser_step = NeroSketchDecoderStepReset;
        }
        break;
    case NeroSketchDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_nero_sketch_const.te_short) <
                subghz_protocol_nero_sketch_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_nero_sketch_const.te_long) <
                subghz_protocol_nero_sketch_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = NeroSketchDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_nero_sketch_const.te_long) <
                 subghz_protocol_nero_sketch_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nero_sketch_const.te_short) <
                 subghz_protocol_nero_sketch_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = NeroSketchDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = NeroSketchDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = NeroSketchDecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_decoder_nero_sketch_serialization(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroSketch* instance = context;

    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse >> 32;
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Yek:0x%lX%08lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        code_found_reverse_lo);
}

bool subghz_protocol_nero_sketch_save_file(void* context, FlipperFile* flipper_file) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroSketch* instance = context;
    return subghz_block_generic_save_file(&instance->generic, flipper_file);
}

bool subghz_protocol_nero_sketch_load_file(
    void* context,
    FlipperFile* flipper_file,
    const char* file_path) {
    furi_assert(context);
    SubGhzProtocolEncoderNeroSketch* instance = context;
    return subghz_block_generic_load_file(&instance->generic, flipper_file);
}

// void subghz_decoder_nero_sketch_to_load_protocol(SubGhzProtocolNeroSketch* instance, void* context) {
//     furi_assert(context);
//     furi_assert(instance);
//     SubGhzProtocolCommonLoad* data = context;
//     instance->common.code_last_found = data->code_found;
//     instance->common.code_last_count_bit = data->code_count_bit;
// }