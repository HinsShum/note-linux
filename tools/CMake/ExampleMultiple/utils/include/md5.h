#ifndef __MD5_H__
#define __MD5_H__

#include <stdint.h>

struct st_md5_ctx
{
	uint32_t count[2];
	uint32_t state[4];
	uint8_t buffer[64];   
};
                                          
void md5_init(struct st_md5_ctx *context);
void md5_update(struct st_md5_ctx *context, uint8_t *input, uint32_t inputlen);
void md5_final(struct st_md5_ctx *context, uint8_t *digest);

#endif  /* __MD5_H__ */
