#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "nbt.h"

static unsigned long region_value(FILE *fp, int size)
{
	unsigned long v = 0;
	while (size--) {
		v <<= 8;
		v |= (unsigned char)fgetc(fp);
	}
	return v;
}

static void *region_inf(void *src, size_t *size)
{
	char buf[4096];
	z_stream strm = {
		.next_in = src,
		.avail_in = *size,
		.total_in = 0,
		.next_out = (void *)buf,
		.avail_out = sizeof(buf),
		.total_out = 0,
		.zalloc = Z_NULL,
		.zfree = Z_NULL,
		.opaque = Z_NULL,
	};
	// +32: Detect gzip or zlib
	int err = inflateInit2(&strm, 32 + MAX_WBITS);
	if (err != Z_OK) {
		inflateEnd(&strm);
		fprintf(stderr, "%s: %s\n", __func__, zError(err));
		return NULL;
	}
	// Multi-step inflation
	void *p = NULL;
	size_t dsize = 0;
	for (;;) {
		err = inflate(&strm, Z_NO_FLUSH);
		if (err != Z_OK && err != Z_STREAM_END) {
			inflateEnd(&strm);
			fprintf(stderr, "%s: %s\n", __func__, zError(err));
			free(p);
			return NULL;
		}
		p = realloc(p, dsize + strm.total_out);
		memcpy(p + dsize, buf, strm.total_out);
		dsize += strm.total_out;
		// Reached end of stream
		if (err == Z_STREAM_END)
			break;
		// More data following
		strm.next_out = (void *)buf;
		strm.avail_out = sizeof(buf);
		strm.total_out = 0;
	}
	inflateEnd(&strm);
	fprintf(stderr, "%s: Inflated %lu bytes to %lu bytes\n",
			__func__, *size, dsize);
	*size = dsize;
	return p;
}

static void *region_chunk(FILE *fp, int x, int z, size_t *size)
{
	int retry = 5;
retry:	if (!retry--)
		return NULL;
	// Chunk offset and sector count
	int fofs = (x + (z << 5)) << 2;
	fseek(fp, fofs, SEEK_SET);
	unsigned long ofs = region_value(fp, 3) << 12;
	unsigned long cnt = region_value(fp, 1) << 12;
	// Chunk timestamp
	fseek(fp, 4096 + fofs, SEEK_SET);
	unsigned long ts = region_value(fp, 4);
	fprintf(stderr, "%s: ofs: %lu, cnt: %lu, ts: %lu\n",
			__func__, ofs, cnt, ts);
	// Chunk data
	fseek(fp, ofs, SEEK_SET);
	unsigned long len = region_value(fp, 4);
	unsigned char type = fgetc(fp);
	if (len > cnt) {
		fprintf(stderr, "%s: Invalid chunk length %lu\n",
				__func__, len);
		goto retry;
	}
	if (type != 1 && type != 2) {
		fprintf(stderr, "%s: Unsupported compression type %u\n",
				__func__, type);
		return NULL;
	}
	void *p = malloc(len);
	if (fread(p, 1, len, fp) != len) {
		fprintf(stderr, "%s: Error reading chunk data\n", __func__);
		fflush(fp);
		free(p);
		goto retry;
	}
	// Check chunk offset and timestamp again to ensure data integrity
	fflush(fp);
	fseek(fp, fofs, SEEK_SET);
	if (region_value(fp, 3) << 12 != ofs ||
			region_value(fp, 1) << 12 != cnt ||
			fseek(fp, 4096 + fofs, SEEK_SET) ||
			region_value(fp, 4) != ts) {
		fprintf(stderr, "%s: Data changed during reading\n", __func__);
		free(p);
		goto retry;
	}
	// Decompress chunk data
	*size = len;
	void *ip = region_inf(p, size);
	free(p);
	return ip;
}

int print(const char *file)
{
	FILE *fp = fopen(file, "rb");
	if (!fp)
		return 1;
	size_t size = 0;
	void *p = region_chunk(fp, 0, 0, &size);
	nbt_dump(nbt_parse(p, size));
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		return 1;
	return print(argv[1]);
}
