#include "pippicore.h"

#define LOGISTIC_SEED_DEFAULT 3.999
#define LOGISTIC_X_DEFAULT 0.555

#define LORENZ_TIMESTEP_DEFAULT 0.01
#define LORENZ_A_DEFAULT 10.0
#define LORENZ_B_DEFAULT 28.0
#define LORENZ_C_DEFAULT (8.0 / 3.0)
#define LORENZ_X_DEFAULT 0.1
#define LORENZ_Y_DEFAULT 0.0
#define LORENZ_Z_DEFAULT 0.0


/* Forward declarations */
void rand_seed(int value);
lpfloat_t rand_base_logistic(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_stdlib(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_rand(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenz(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenzX(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenzY(lpfloat_t low, lpfloat_t high);
lpfloat_t rand_base_lorenzZ(lpfloat_t low, lpfloat_t high);
int rand_randint(int low, int high);
int rand_randbool(void);
int rand_choice(int numchoices);

lparray_t * create_array_from(int numvalues, ...);
lparray_t * create_array(size_t length);
void destroy_array(lparray_t * array);

lpbuffer_t * create_buffer(size_t length, int channels, int samplerate);
void copy_buffer(lpbuffer_t * src, lpbuffer_t * dest);
void scale_buffer(lpbuffer_t * buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max);
void multiply_buffer(lpbuffer_t * a, lpbuffer_t * b);
void scalar_multiply_buffer(lpbuffer_t * a, lpfloat_t b);
void add_buffers(lpbuffer_t * a, lpbuffer_t * b);
void scalar_add_buffer(lpbuffer_t * a, lpfloat_t b);
void subtract_buffers(lpbuffer_t * a, lpbuffer_t * b);
void scalar_subtract_buffer(lpbuffer_t * a, lpfloat_t b);
void divide_buffers(lpbuffer_t * a, lpbuffer_t * b);
void scalar_divide_buffer(lpbuffer_t * a, lpfloat_t b);
lpbuffer_t * concat_buffers(lpbuffer_t * a, lpbuffer_t * b);
int buffers_are_equal(lpbuffer_t * a, lpbuffer_t * b);
int buffers_are_close(lpbuffer_t * a, lpbuffer_t * b, int d);
void dub_buffer(lpbuffer_t * a, lpbuffer_t * b);
void env_buffer(lpbuffer_t * buf, lpbuffer_t * env);
lpfloat_t play_buffer(lpbuffer_t * buf, lpfloat_t speed);
void copy_buffer(lpbuffer_t * src, lpbuffer_t * dest);
lpbuffer_t * mix_buffers(lpbuffer_t * a, lpbuffer_t * b);
void destroy_buffer(lpbuffer_t * buf);
void destroy_stack(lpstack_t * stack);

lpbuffer_t * ringbuffer_create(size_t length, int channels, int samplerate);
void ringbuffer_fill(lpbuffer_t * ringbuf, lpbuffer_t * buf, int offset);
lpfloat_t ringbuffer_readone(lpbuffer_t * ringbuf, int offset);
lpbuffer_t * ringbuffer_read(lpbuffer_t * ringbuf, size_t length);
void ringbuffer_readinto(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels);
void ringbuffer_writeone(lpbuffer_t * ringbuf, lpfloat_t sample);
void ringbuffer_writefrom(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels);
void ringbuffer_write(lpbuffer_t * ringbuf, lpbuffer_t * buf);
void ringbuffer_dub(lpbuffer_t * buf, lpbuffer_t * src);
void ringbuffer_destroy(lpbuffer_t * buf);

void memorypool_init(unsigned char * pool, size_t poolsize);
lpmemorypool_t * memorypool_custom_init(unsigned char * pool, size_t poolsize);
void * memorypool_alloc(size_t itemcount, size_t itemsize);
void * memorypool_custom_alloc(lpmemorypool_t * pool, size_t itemcount, size_t itemsize);
void memorypool_free(void * ptr);

lpfloat_t interpolate_hermite(lpbuffer_t * buf, lpfloat_t phase);
lpfloat_t interpolate_hermite_pos(lpbuffer_t * buf, lpfloat_t pos);
lpfloat_t interpolate_linear(lpbuffer_t * buf, lpfloat_t phase);
lpfloat_t interpolate_linear_pos(lpbuffer_t * buf, lpfloat_t pos);

lpbuffer_t * param_create_from_float(lpfloat_t value);
lpbuffer_t * param_create_from_int(int value);

lpbuffer_t * create_wavetable(int name, size_t length);
lpstack_t * create_wavetable_stack(int numtables, ...);
void destroy_wavetable(lpbuffer_t* buf);
lpbuffer_t* create_window(int name, size_t length);
lpstack_t * create_window_stack(int numtables, ...);
void destroy_window(lpbuffer_t* buf);

/* Populate interfaces */
lprand_t LPRand = { LOGISTIC_SEED_DEFAULT, LOGISTIC_X_DEFAULT, \
    LORENZ_TIMESTEP_DEFAULT, \
    LORENZ_X_DEFAULT, LORENZ_Y_DEFAULT, LORENZ_Z_DEFAULT, \
    LORENZ_A_DEFAULT, LORENZ_B_DEFAULT, LORENZ_C_DEFAULT, \
    rand_seed, rand_base_stdlib, rand_base_logistic, \
    rand_base_lorenz, rand_base_lorenzX, rand_base_lorenzY, rand_base_lorenzZ, \
    rand_base_stdlib, rand_rand, rand_randint, rand_randbool, rand_choice };
lpmemorypool_factory_t LPMemoryPool = { 0, 0, 0, memorypool_init, memorypool_custom_init, memorypool_alloc, memorypool_custom_alloc, memorypool_free };
const lparray_factory_t LPArray = { create_array, create_array_from, destroy_array };
const lpbuffer_factory_t LPBuffer = { create_buffer, copy_buffer, scale_buffer, play_buffer, mix_buffers, multiply_buffer, scalar_multiply_buffer, add_buffers, scalar_add_buffer, subtract_buffers, scalar_subtract_buffer, divide_buffers, scalar_divide_buffer, concat_buffers, buffers_are_equal, buffers_are_close, dub_buffer, env_buffer, destroy_buffer, destroy_stack };
const lpinterpolation_factory_t LPInterpolation = { interpolate_linear_pos, interpolate_linear, interpolate_hermite_pos, interpolate_hermite };
const lpparam_factory_t LPParam = { param_create_from_float, param_create_from_int };
const lpwavetable_factory_t LPWavetable = { create_wavetable, create_wavetable_stack, destroy_wavetable };
const lpwindow_factory_t LPWindow = { create_window, create_window_stack, destroy_window };
const lpringbuffer_factory_t LPRingBuffer = { ringbuffer_create, ringbuffer_fill, ringbuffer_read, ringbuffer_readinto, ringbuffer_writefrom, ringbuffer_write, ringbuffer_readone, ringbuffer_writeone, ringbuffer_dub, ringbuffer_destroy };

/** Rand
 */
void rand_seed(int value) {
    srand((unsigned int)value);
}

lpfloat_t rand_base_stdlib(lpfloat_t low, lpfloat_t high) {
    return (rand()/(lpfloat_t)RAND_MAX) * (high-low) + low;
}

lpfloat_t rand_base_logistic(lpfloat_t low, lpfloat_t high) {
    LPRand.logistic_x = LPRand.logistic_seed * LPRand.logistic_x * (1.f - LPRand.logistic_x);
    return LPRand.logistic_x * (high-low) + low;
}

/* The three Lorenz attractor implementations (lorenzX, lorenzY, lorenzZ) 
 * were lightly adapted with permission from Greg Cope's helpful overview: 
 *      https://www.algosome.com/articles/lorenz-attractor-programming-code.html
 * Please consider those routines to be included here under an MIT license.
 */
lpfloat_t lorenzX(lpfloat_t low, lpfloat_t high) {
    LPRand.lorenz_x = LPRand.lorenz_x + LPRand.lorenz_timestep * LPRand.lorenz_a * (LPRand.lorenz_y - LPRand.lorenz_x);
    return LPRand.lorenz_x * (high-low) + low;
}

lpfloat_t lorenzY(lpfloat_t low, lpfloat_t high) {
    LPRand.lorenz_y = LPRand.lorenz_y + LPRand.lorenz_timestep * (LPRand.lorenz_x * (LPRand.lorenz_b - LPRand.lorenz_z) - LPRand.lorenz_y);
    return LPRand.lorenz_y * (high-low) + low;
}

lpfloat_t lorenzZ(lpfloat_t low, lpfloat_t high) {
    LPRand.lorenz_z = LPRand.lorenz_z + LPRand.lorenz_timestep * (LPRand.lorenz_x * LPRand.lorenz_y - LPRand.lorenz_c * LPRand.lorenz_z);
    return LPRand.lorenz_z * (high-low) + low;
}

lpfloat_t rand_base_lorenzX(lpfloat_t low, lpfloat_t high) {
    lpfloat_t x;
    x = lorenzX(0.f, 1.f);
    lorenzY(0.f, 1.f);
    lorenzZ(0.f, 1.f);
    return x * (high-low) + low;
}

lpfloat_t rand_base_lorenzY(lpfloat_t low, lpfloat_t high) {
    lpfloat_t y;
    lorenzX(0.f, 1.f);
    y = lorenzY(0.f, 1.f);
    lorenzZ(0.f, 1.f);
    return y * (high-low) + low;
}

lpfloat_t rand_base_lorenzZ(lpfloat_t low, lpfloat_t high) {
    lpfloat_t z;
    lorenzX(0.f, 1.f);
    lorenzY(0.f, 1.f);
    z = lorenzZ(0.f, 1.f);
    return z * (high-low) + low;
}

lpfloat_t rand_base_lorenz(lpfloat_t low, lpfloat_t high) {
    lpfloat_t x, y, z;
    x = lorenzX(0.f, 1.f);
    y = lorenzY(0.f, 1.f);
    z = lorenzZ(0.f, 1.f);
    return x * y * z * (high-low) + low;
}

lpfloat_t rand_rand(lpfloat_t low, lpfloat_t high) {
    return LPRand.rand_base(low, high);
}

int rand_randint(int low, int high) {
    float diff, tmp;

    tmp = (float)rand_rand((lpfloat_t)low, (lpfloat_t)high);
    diff = (int)tmp - tmp;

    if(diff >= 0.5f) {
        return (int)ceil(tmp);
    } else {
        return (int)floor(tmp);
    }
}

int rand_randbool(void) {
    return rand_randint(0, 1);
}

int rand_choice(int numchoices) {
    assert(numchoices > 0);
    if(numchoices == 1) return 0;
    return rand_randint(0, numchoices-1);
}

lparray_t * create_array(size_t length) {
    int i = 0;
    lparray_t * array = (lparray_t*)LPMemoryPool.alloc(1, sizeof(lparray_t));
    array->data = (int*)LPMemoryPool.alloc(length, sizeof(int));
    array->length = length;
    for(i=0; i < array->length; i++) {
        array->data[i] = 0;
    }
    array->phase = 0.f;
    return array;
}

lparray_t * create_array_from(int numvalues, ...) {
    va_list vl;
    lparray_t * array;
    int i;

    va_start(vl, numvalues);

    array = (lparray_t*)LPMemoryPool.alloc(1, sizeof(lparray_t));
    array->data = (int*)LPMemoryPool.alloc(numvalues, sizeof(int));

    for(i=0; i < numvalues; i++) {
        array->data[i] = va_arg(vl, int);
    }

    va_end(vl);

    return array;
}

void destroy_array(lparray_t * array) {
    if(array != NULL) {
        if(array->data != NULL) {
            LPMemoryPool.free(array->data);
        }
        LPMemoryPool.free(array);
    }
}

/* Buffer
 * */
lpbuffer_t * create_buffer(size_t length, int channels, int samplerate) {
    lpbuffer_t * buf;
    buf = (lpbuffer_t*)LPMemoryPool.alloc(1, sizeof(lpbuffer_t));
    buf->data = (lpfloat_t*)LPMemoryPool.alloc(length * channels, sizeof(lpfloat_t));
    buf->channels = channels;
    buf->length = length;
    buf->samplerate = samplerate;
    buf->phase = 0.f;
    buf->pos = 0;
    buf->range = length;
    return buf;
}

void copy_buffer(lpbuffer_t * src, lpbuffer_t * dest) {
    size_t i, c;

    assert(src->length == dest->length);
    assert(src->channels == dest->channels);

    for(i=0; i < src->length; i++) {
        for(c=0; c < src->channels; c++) {
            dest->data[i * src->channels + c] = src->data[i * src->channels + c];
        }
    }
}

void scale_buffer(lpbuffer_t * buf, lpfloat_t from_min, lpfloat_t from_max, lpfloat_t to_min, lpfloat_t to_max) {
    size_t i, c;
    int idx;
    lpfloat_t from_diff, to_diff;

    to_diff = to_max - to_min;;
    from_diff = from_max - from_min;;

    /* Maybe this is valid? It's weird to "scale" 
     * a buffer filled with one value, but I guess 
     * that's a case we should support...
     * Ideally we'll figure out how to get rid of that 
     * repeating divide and use an approach that supports 
     * this case.
     * Anyway: TODO handle this better?
     */
    assert(from_diff != 0);

    for(i=0; i < buf->length; i++) {
        for(c=0; c < buf->channels; c++) {
            idx = i * buf->channels + c;
            buf->data[idx] = ((buf->data[idx] - from_min) / from_diff) * to_diff + to_min;
        }
    }
}

void pan_buffer(lpbuffer_t * buf, lpbuffer_t * pos) {

}

lpfloat_t play_buffer(lpbuffer_t * buf, lpfloat_t speed) {
    lpfloat_t phase_inc, value;
    phase_inc = 1.f / (buf->length * (1.f / speed));
    value = interpolate_linear_pos(buf, buf->phase);
    buf->phase += phase_inc;
    return value;
}

lpbuffer_t * resample_buffer(lpbuffer_t * buf, size_t length) {
    lpbuffer_t * out;
    lpfloat_t pos;
    int i, c;

    assert(length > 1);
    out = create_buffer(length, buf->channels, buf->samplerate);
    for(i=0; i < length; i++) {
        pos = (lpfloat_t)i / length;
        for(c=0; c < buf->channels; c++) {
            out->data[i * buf->channels + c] = play_buffer(buf, pos);
        }
    }

    return out;
}

void multiply_buffer(lpbuffer_t * a, lpbuffer_t * b) {
    size_t length, i, c, j;
    length = (a->length <= b->length) ? a->length : b->length;
    for(i=0; i < length; i++) {
        for(c=0; c < a->channels; c++) {
            j = c % b->channels;
            a->data[i * a->channels + c] *= b->data[i * b->channels + j];
        }
    }
}

void scalar_multiply_buffer(lpbuffer_t * a, lpfloat_t b) {
    size_t i, c;
    for(i=0; i < a->length; i++) {
        for(c=0; c < a->channels; c++) {
            a->data[i * a->channels + c] *= b;
        }
    }
}

lpbuffer_t * concat_buffers(lpbuffer_t * a, lpbuffer_t * b) {
    size_t length, i;
    int c, channels;
    lpbuffer_t * out;

    length = a->length + b->length;
    channels = a->channels;
    out = create_buffer(length, channels, a->samplerate);

    for(i=0; i < a->length; i++) {
        for(c=0; c < channels; c++) {
            out->data[i * channels + c] = a->data[i * channels + c];
        }
    }

    for(i=a->length; i < length; i++) {
        for(c=0; c < channels; c++) {
            out->data[i * channels + c] = b->data[i * channels + c];
        }
    }

    return out;
}

void add_buffers(lpbuffer_t * a, lpbuffer_t * b) {
    size_t length, i, c, j;
    length = (a->length <= b->length) ? a->length : b->length;
    for(i=0; i < length; i++) {
        for(c=0; c < a->channels; c++) {
            j = c % b->channels;
            a->data[i * a->channels + c] += b->data[i * b->channels + j];
        }
    }
}

void scalar_add_buffer(lpbuffer_t * a, lpfloat_t b) {
    size_t i, c;
    for(i=0; i < a->length; i++) {
        for(c=0; c < a->channels; c++) {
            a->data[i * a->channels + c] += b;
        }
    }
}

void subtract_buffers(lpbuffer_t * a, lpbuffer_t * b) {
    size_t length, i, c, j;
    length = (a->length <= b->length) ? a->length : b->length;
    for(i=0; i < length; i++) {
        for(c=0; c < a->channels; c++) {
            j = c % b->channels;
            a->data[i * a->channels + c] -= b->data[i * b->channels + j];
        }
    }
}

void scalar_subtract_buffer(lpbuffer_t * a, lpfloat_t b) {
    size_t i, c;
    for(i=0; i < a->length; i++) {
        for(c=0; c < a->channels; c++) {
            a->data[i * a->channels + c] -= b;
        }
    }
}

void divide_buffers(lpbuffer_t * a, lpbuffer_t * b) {
    size_t length, i, c, j;
    length = (a->length <= b->length) ? a->length : b->length;
    for(i=0; i < length; i++) {
        for(c=0; c < a->channels; c++) {
            j = c % b->channels;
            if(b->data[i * b->channels + j] == 0) {
                a->data[i * a->channels + c] = 0.f;
            } else {
                a->data[i * a->channels + c] /= b->data[i * b->channels + j];
            }
        }
    }
}

void scalar_divide_buffer(lpbuffer_t * a, lpfloat_t b) {
    size_t i, c;
    if(b == 0) {
        for(i=0; i < a->length; i++) {
            for(c=0; c < a->channels; c++) {
                a->data[i * a->channels + c] = 0.f;
            }
        }
    } else {
        for(i=0; i < a->length; i++) {
            for(c=0; c < a->channels; c++) {
                a->data[i * a->channels + c] /= b;
            }
        }
    }
}

int buffers_are_equal(lpbuffer_t * a, lpbuffer_t * b) {
    size_t i, c;
    if(a->length != b->length) return 0;
    if(a->channels != b->channels) return 0;
    for(i=0; i < a->length; i++) {
        for(c=0; c < a->channels; c++) {
            if(a->data[i * a->channels + c] != b->data[i * a->channels + c]) return 0;
        }
    }
    return 1;
}

int buffers_are_close(lpbuffer_t * a, lpbuffer_t * b, int d) {
    size_t i, c;
    long atmp, btmp;
    if(a->length != b->length) return 0;
    if(a->channels != b->channels) return 0;
    for(i=0; i < a->length; i++) {
        for(c=0; c < a->channels; c++) {
            atmp = floor(a->data[i * a->channels + c] * d);
            btmp = floor(b->data[i * a->channels + c] * d);
            if(atmp != btmp) return 0;
        }
    }
    return 1;
}


void env_buffer(lpbuffer_t * buf, lpbuffer_t * env) {
    lpfloat_t pos, value;
    size_t i, c;

    assert(env->length > 0);

    for(i=0; i < buf->length; i++) {
        pos = (lpfloat_t)i / buf->length;
        for(c=0; c < buf->channels; c++) {
            value = interpolate_linear_pos(env, pos);
            buf->data[i * buf->channels + c] *= value;
        }
    }
}

void dub_buffer(lpbuffer_t * a, lpbuffer_t * b) {
    size_t length, i, c, j;
    length = (a->length <= b->length) ? a->length : b->length;
    for(i=0; i < length; i++) {
        for(c=0; c < a->channels; c++) {
            j = b->channels % c;
            a->data[i * a->channels + c] += b->data[i * b->channels + j];
        }
    }
}

lpbuffer_t * mix_buffers(lpbuffer_t * a, lpbuffer_t * b) {
    int max_channels, max_samplerate, i, c;
    lpbuffer_t * out;
    lpbuffer_t * longest;
    lpbuffer_t * shortest;

    if(a->length >= b->length) {
        longest=a; shortest=b;
    } else {
        longest=b; shortest=a;
    }

    max_channels = (a->channels >= b->channels) ? a->channels : b->channels;
    max_samplerate = (a->samplerate >= b->samplerate) ? a->samplerate : b->samplerate;
    out = LPBuffer.create(longest->length, max_channels, max_samplerate);

    for(i=0; i < longest->length; i++) {
        for(c=0; c < max_channels; c++) {
            out->data[i * max_channels + c] = longest->data[i * max_channels + c];
        }
    }

    for(i=0; i < shortest->length; i++) {
        for(c=0; c < max_channels; c++) {
            out->data[i * max_channels + c] = shortest->data[i * max_channels + c];
        }
    }

    return out;
}

void destroy_buffer(lpbuffer_t * buf) {
    if(buf != NULL) {
        LPMemoryPool.free(buf->data);
        LPMemoryPool.free(buf);
    }
}

void destroy_stack(lpstack_t * stack) {
    int i;
    if(stack->stack != NULL) {
        for(i=0; i < stack->length; i++) {
            LPBuffer.destroy(stack->stack[i]);
        }
        /*LPMemoryPool.free(stack->stack);*/
    }
    LPMemoryPool.free(stack);
}


/* RingBuffers
 */
lpbuffer_t * ringbuffer_create(size_t length, int channels, int samplerate) {
    lpbuffer_t * ringbuf;
    ringbuf = LPBuffer.create(length, channels, samplerate);
    ringbuf->pos = 0;
    ringbuf->boundry = length - 1;
    return ringbuf;
}

void ringbuffer_fill(lpbuffer_t * ringbuf, lpbuffer_t * buf, int offset) {
    int i, c;
    size_t pos = ringbuf->pos - buf->length - offset;
    pos = pos % ringbuf->length;
    for(i=0; i < buf->length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            buf->data[i * buf->channels + c] = ringbuf->data[pos * ringbuf->channels + c];
        }

        pos += 1;
        pos = pos % ringbuf->length;
    }
}

lpfloat_t ringbuffer_readone(lpbuffer_t * ringbuf, int offset) {
    return ringbuf->data[(ringbuf->pos - offset) % ringbuf->length];
}

void ringbuffer_readinto(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels) {
    int i, c;
    size_t pos = ringbuf->pos - length;
    pos = pos % ringbuf->length;

    for(i=0; i < length; i++) {
        for(c=0; c < channels; c++) {
            data[i * channels + c] = ringbuf->data[pos * channels + c];
        }

        pos += 1;
        pos = pos % ringbuf->length;
    }
}

lpbuffer_t * ringbuffer_read(lpbuffer_t * ringbuf, size_t length) {
    int i, c;
    size_t pos = ringbuf->pos - length;
    lpbuffer_t * out;

    pos = pos % ringbuf->length;
    out = LPBuffer.create(length, ringbuf->channels, ringbuf->samplerate);
    for(i=0; i < length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            out->data[i * out->channels + c] = ringbuf->data[pos * ringbuf->channels + c];
        }

        pos += 1;
        pos = pos % ringbuf->length;
    }

    return out;
}

void ringbuffer_writeone(lpbuffer_t * ringbuf, lpfloat_t sample) {
    ringbuf->data[ringbuf->pos] = sample;
    ringbuf->pos += 1;
    ringbuf->pos = ringbuf->pos % ringbuf->length;
}

void ringbuffer_writefrom(lpbuffer_t * ringbuf, lpfloat_t * data, int length, int channels) {
    int i, c, j;
    for(i=0; i < length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            j = c % channels;
            ringbuf->data[ringbuf->pos * ringbuf->channels + c] = data[i * channels + j];
        }

        ringbuf->pos += 1;
        ringbuf->pos = ringbuf->pos % ringbuf->length;
    }
}

void ringbuffer_write(lpbuffer_t * ringbuf, lpbuffer_t * buf) {
    int i, c, j;
    for(i=0; i < buf->length; i++) {
        for(c=0; c < ringbuf->channels; c++) {
            j = c % buf->channels;
            ringbuf->data[ringbuf->pos * ringbuf->channels + c] = buf->data[i * buf->channels + j];
        }

        ringbuf->pos += 1;
        ringbuf->pos = ringbuf->pos % ringbuf->length;
    }
}

void ringbuffer_dub(lpbuffer_t * buf, lpbuffer_t * src) {
    int i, c, j;
    for(i=0; i < src->length; i++) {
        for(c=0; c < buf->channels; c++) {
            j = c % src->channels;
            buf->data[buf->pos * buf->channels + c] += src->data[i * src->channels + j];
        }

        buf->pos += 1;
        buf->pos = buf->pos % buf->length;
    }
}

void ringbuffer_destroy(lpbuffer_t * buf) {
    LPBuffer.destroy(buf);
}


/* LPMemoryPool
 * */
void memorypool_init(unsigned char * pool, size_t poolsize) {
    assert(poolsize >= 1);
    LPMemoryPool.pool = pool;
    LPMemoryPool.poolsize = poolsize;
    LPMemoryPool.pos = 0;
}

lpmemorypool_t * memorypool_custom_init(unsigned char * pool, size_t poolsize) {
    lpmemorypool_t * mp;
    mp = (lpmemorypool_t *)LPMemoryPool.alloc(1, sizeof(lpmemorypool_t));

    assert(poolsize >= 1);
    mp->pool = pool;
    mp->poolsize = poolsize;
    mp->pos = 0;

    return mp;
}

void * memorypool_custom_alloc(lpmemorypool_t * mp, size_t itemcount, size_t itemsize) {
    void * p;
    size_t length;

    assert(mp->pool != 0); 
    length = itemcount * itemsize;

    if(mp->poolsize >= mp->pos + length) {
        p = (void *)(&mp->pool[mp->pos]);
        mp->pos += length;
        return p;
    }
    /* FIXME might as well try to expand the pool here */
    exit(EXIT_FAILURE);
}

void * memorypool_alloc(size_t itemcount, size_t itemsize) {
#ifdef LP_STATIC
    void * p;
    size_t length;

    assert(LPMemoryPool.pool != 0); 
    length = itemcount * itemsize;

    if(LPMemoryPool.poolsize >= LPMemoryPool.pos + length) {
        p = (void *)(&LPMemoryPool.pool[LPMemoryPool.pos]);
        LPMemoryPool.pos += length;
        return p;
    }
    exit(EXIT_FAILURE);
#else
    return calloc(itemcount, itemsize);
#endif
}

void memorypool_free(void * ptr) {
#ifndef LP_STATIC
    if(ptr != NULL) free(ptr);
#endif
}

/* Param
 * */
lpbuffer_t * param_create_from_float(lpfloat_t value) {
    lpbuffer_t * param = create_buffer(1, 1, DEFAULT_SAMPLERATE);
    param->data[0] = value;
    return param;
}

lpbuffer_t * param_create_from_int(int value) {
    lpbuffer_t * param = create_buffer(1, 1, DEFAULT_SAMPLERATE);
    param->data[0] = (lpfloat_t)value;
    return param;
}

/* Interpolation
 * */
lpfloat_t interpolate_hermite(lpbuffer_t* buf, lpfloat_t phase) {
    lpfloat_t y0, y1, y2, y3, frac;
    lpfloat_t c0, c1, c2, c3;
    int i0, i1, i2, i3, boundry;

    if(buf->length == 1) return buf->data[0];
    if(buf->length < 1) return 0;

    boundry = buf->length - 1;

    frac = phase - (int)phase;
    i1 = (int)phase;
    i2 = i1 + 1;
    i3 = i2 + 1;
    i0 = i1 - 1;

    y0 = 0;
    y1 = 0;
    y2 = 0;
    y3 = 0;

    if(i0 >= 0) y0 = buf->data[i0];
    if(i1 <= boundry) y1 = buf->data[i1];
    if(i2 <= boundry) y2 = buf->data[i2];
    if(i3 <= boundry) y3 = buf->data[i3];

    /* This part was taken from version #2 by James McCartney 
     * https://www.musicdsp.org/en/latest/Other/93-hermite-interpollation.html
     */
    c0 = y1;
    c1 = 0.5 * (y2 - y0);
    c3 = 1.5 * (y1 - y2) + 0.5 * (y3 - y0);
    c2 = y0 - y1 + c1 - c3;
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

lpfloat_t interpolate_hermite_pos(lpbuffer_t* buf, lpfloat_t pos) {
    return interpolate_hermite(buf, pos * buf->length);
}

lpfloat_t interpolate_linear(lpbuffer_t* buf, lpfloat_t phase) {
    lpfloat_t frac, a, b;
    size_t i;

    if(buf->length == 1) return buf->data[0];
    
    frac = phase - (int)phase;
    i = (int)phase;

    if (i >= buf->length-1 || i < 0) return 0;

    a = buf->data[i];
    b = buf->data[i+1];

    return (1.0 - frac) * a + (frac * b);
}

lpfloat_t interpolate_linear_pos(lpbuffer_t* buf, lpfloat_t pos) {
    return interpolate_linear(buf, pos * buf->length);
}

/* Wavetable generators
 * 
 * All these functions return a table of values 
 * of the given length with values between -1 and 1
 */
void wavetable_sine(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = sin((i/(lpfloat_t)length) * PI * 2.0);         
    }
}

void wavetable_square(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        if(i < (length/2.0)) {
            out[i] = 0.9999;
        } else {
            out[i] = -0.9999;
        }
    }
}

void wavetable_tri(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = fabs((i/(lpfloat_t)length) * 2.0 - 1.0) * 2.0 - 1.0;      
    }
}


/* create a wavetable (-1 to 1) */
lpbuffer_t* create_wavetable(int name, size_t length) {
    lpbuffer_t* buf = LPBuffer.create(length, 1, -1);
    if(name == WT_SINE) {
        wavetable_sine(buf->data, length);            
    } else if (name == WT_TRI) {
        wavetable_tri(buf->data, length);            
    } else if (name == WT_SQUARE) {
        wavetable_square(buf->data, length);            
    } else if (name == WT_RND) {
        return create_wavetable(rand_choice(NUM_WAVETABLES), length);
    } else {
        wavetable_sine(buf->data, length);            
    }
    return buf;
}

lpstack_t * create_wavetable_stack(int numtables, ...) {
    va_list vl;
    lpstack_t * stack;
    int i, name;

    va_start(vl, numtables);

    stack = (lpstack_t *)LPMemoryPool.alloc(1, sizeof(lpstack_t));
    stack->stack = (lpbuffer_t **)LPMemoryPool.alloc(numtables, sizeof(lpbuffer_t *));
    stack->length = numtables;
    stack->phase = 0.f;
    stack->pos = 0.f;

    for(i=0; i < numtables; i++) {
        name = va_arg(vl, int);
        stack->stack[i] = LPWavetable.create(name, DEFAULT_TABLESIZE); 
    }

    va_end(vl);

    return stack;
}


void destroy_wavetable(lpbuffer_t* buf) {
    LPBuffer.destroy(buf);
}


/* Window generators
 *
 * All these functions return a table of values 
 * of the given length with values between 0 and 1
 */
void window_phasor(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = i/(lpfloat_t)length;      
    }
}

void window_tri(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = fabs((i/(lpfloat_t)length) * 2.0 - 1.0);      
    }
}

void window_sine(lpfloat_t* out, int length) {
    int i;
    for(i=0; i < length; i++) {
        out[i] = sin((i/(lpfloat_t)length) * PI);         
    }
}

void window_hanning(lpfloat_t* out, int length) {
    int i;
    assert(length > 1);
    for(i=0; i < length; i++) {
        out[i] = 0.5 - 0.5 * cos(2.0 * PI * i / (length-1.0));
    }
}


/* create a window (0 to 1) */
lpbuffer_t * create_window(int name, size_t length) {
    lpbuffer_t* buf = LPBuffer.create(length, 1, -1);
    if(name == WIN_SINE) {
        window_sine(buf->data, length);            
    } else if (name == WIN_TRI) {
        window_tri(buf->data, length);            
    } else if (name == WIN_PHASOR) {
        window_phasor(buf->data, length);            
    } else if (name == WIN_HANN) {
        window_hanning(buf->data, length);            
    } else if (name == WIN_RND) {
        return create_window(rand_choice(NUM_WINDOWS), length);
    } else {
        window_sine(buf->data, length);            
    }
    return buf;
}

lpstack_t * create_window_stack(int numtables, ...) {
    va_list vl;
    lpstack_t * stack;
    int i, name;

    va_start(vl, numtables);

    stack = (lpstack_t *)LPMemoryPool.alloc(1, sizeof(lpstack_t));
    stack->stack = (lpbuffer_t **)LPMemoryPool.alloc(numtables, sizeof(lpbuffer_t *));
    stack->length = numtables;
    stack->phase = 0.f;
    stack->pos = 0.f;

    for(i=0; i < numtables; i++) {
        name = va_arg(vl, int);
        stack->stack[i] = LPWindow.create(name, DEFAULT_TABLESIZE); 
    }

    va_end(vl);

    return stack;
}

void destroy_window(lpbuffer_t* buf) {
    LPBuffer.destroy(buf);
}

/* Utilities */

/* The zapgremlins() routine was written by James McCartney as part of SuperCollider:
 * https://github.com/supercollider/supercollider/blob/f0d4f47a33b57b1f855fe9ca2d4cb427038974f0/headers/plugin_interface/SC_InlineUnaryOp.h#L35
 *
 * SuperCollider real time audio synthesis system
 * Copyright (c) 2002 James McCartney. All rights reserved.
 * http://www.audiosynth.com
 *
 * He says:
 *      This is a function for preventing pathological math operations in ugens.
 *      It can be used at the end of a block to fix any recirculating filter values.
 */
lpfloat_t lpzapgremlins(lpfloat_t x) {
    lpfloat_t absx;
    absx = fabs(x);
    return (absx > (lpfloat_t)1e-15 && absx < (lpfloat_t)1e15) ? x : (lpfloat_t)0.f;
}

lpfloat_t lpwv(lpfloat_t value, lpfloat_t min, lpfloat_t max) {
    /* wrap value */
    if(value > max) value -= max;
    if(value < min) value += min;
    return value;
}

lpfloat_t lpsv(lpfloat_t value, lpfloat_t min, lpfloat_t max) {
    /* scale value */
    return value * (max-min) + min;
}

lpfloat_t lpsvf(lpfloat_t value, lpfloat_t min, lpfloat_t max, lpfloat_t from, lpfloat_t to) {
    /* scale value from (a range other than 0-1) */
    lpfloat_t delta = to - from;
    if(delta <= 0) return 0;
    return (value/delta) * (max-min) + min;
}