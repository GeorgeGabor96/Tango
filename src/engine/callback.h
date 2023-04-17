#ifndef __ENGINE_CALLBACK_H__
#define __ENGINE_CALLBACK_H__


/**********************
* Callback Meta Dumper
**********************/
typedef struct DumperMeta {
    String* meta_file;
    u32 sample_duration;
    u32 sample_time;
} DumperMeta;


/**********************
* Callback definitions
**********************/
typedef enum {
    CALLBACK_META_DUMPER = 0,

    CALLBACK_INVALID,
} CallbackType;

internal char* callback_type_get_c_str(CallbackType type);


typedef struct Callback {
    CallbackType type;

    union {
        DumperMeta dumper_meta;
    };
} Callback, *CallbackP;


/********************
* Callback functions
********************/
internal void callback_begin_sample(Callback* callback,
                                    u32 sample_duration,
                                    Memory* memory);
internal void callback_update(Callback* callback, Memory* memory);
internal void callback_end_sample(Callback* callback, Memory* memory);


/**********************
* Callback Meta Dumper
**********************/
internal Callback* callback_meta_dumper_create(
    Memory* memory,
    String* output_folder,
    Network* network);


internal void callback_meta_dumper_begin_sample(
    Callback* callback,
    u32 sample_duration,
    Memory* memory);


internal void callback_meta_dumper_update(
    Callback* callback,
    Memory* memory);


internal void callback_meta_dumper_end_sample(
    Callback* callback,
    Memory* memory);



#endif // __ENGINE_CALLBACK_H__
