#ifndef _BIT_FIELD_H_
#define _BIT_FIELD_H_

/**
 * the max value of bit field "[start, end]"
 */
static inline unsigned long bit_field_max(int start, int end)
{
    return (1ul << (end - start + 1)) - 1;
}

/**
 * the mask of bit field
 * mask = bit_field_max(start, end) << start;
 */
static inline unsigned long bit_field_mask(int start, int end)
{
    return bit_field_max(start, end) << start;
}

/**
 * check value is valid for the bit field
 * return bit_field_max(start, end) >= val;
 */
static inline int check_bit_field(int start, int end, unsigned int val)
{
    return bit_field_max(start, end) >= val;
}

/**
 * set value to the bit field
 * reg[start, end] = val;
 * @attention: this function do not check the value is valid for the bit field or not
 */
static inline void set_bit_field(volatile unsigned int *reg, int start, int end, unsigned int val)
{
    *reg = (*reg & ~bit_field_mask(start, end)) | (val << start);
}

/**
 * get value in the bit field
 * return reg[start, end];
 */
static inline unsigned int get_bit_field(volatile unsigned int *reg, int start, int end, unsigned int val)
{
    return (*reg & bit_field_mask(start, end)) >> start;
}

#endif /* _BIT_FIELD_H_ */
