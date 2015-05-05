#include <stdio.h>
#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "zlib.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

// WARNING WARNING DANGER FIXED BUFFER SIZE THAT WAS AN ADEQUATE UPPER
// BOUND FOR MY PURPOSES DO NOT USE IN PRODUCTION
#define BUF_SIZE 300000

// modified from https://github.com/madler/zlib/blob/master/uncompr.c
// I just changed
// inflateInit(&stream);
// to
// inflateInit2(&stream, 31);
// Because the 31 = 16 + 15 indicates gzip format (16) with 15 window bits
// according to
// https://github.com/madler/zlib/blob/master/zlib.h
int gzuncompress (dest, destLen, source, sourceLen)
    Bytef *dest;
    uLongf *destLen;
    const Bytef *source;
    uLong sourceLen;
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef *)source;
    stream.avail_in = (uInt)sourceLen;
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    err = inflateInit2(&stream, 31);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
            return Z_DATA_ERROR;
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}

Datum gunzip(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(gunzip);
Datum gunzip(PG_FUNCTION_ARGS) {
   bytea *gzipped_input = PG_GETARG_BYTEA_P(0);
   unsigned char *src_ptr = (unsigned char *) VARDATA(gzipped_input);
   uLongf destLen = BUF_SIZE;
   text *destination = (text *) palloc(VARHDRSZ + BUF_SIZE);
   unsigned char *dst_ptr = (unsigned char *) VARDATA(destination);

   gzuncompress (dst_ptr, &destLen,
                 src_ptr, VARSIZE(gzipped_input) - VARHDRSZ);

   SET_VARSIZE(destination, VARHDRSZ + destLen);

   PG_RETURN_TEXT_P(destination);
}
