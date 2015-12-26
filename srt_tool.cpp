// srt_tool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int32_t conv_time_str_2_ms(const char *time_str)
{
	int32_t hh = (time_str[0] - '0') * 10 + (time_str[1] - '0');
	int32_t mm = (time_str[3] - '0') * 10 + (time_str[4] - '0');
	int32_t ss = (time_str[6] - '0') * 10 + (time_str[7] - '0');
	int32_t ms = (time_str[9] - '0') * 100 + (time_str[10] - '0') * 10 + (time_str[11] - '0');

	return (hh * 3600 + mm * 60 + ss) * 1000 + ms;
}

void conv_time_ms_2_str(int32_t ms_val, char *time_str)
{
	int32_t ss_val = ms_val / 1000; ms_val %= 1000;
	int32_t hh_val = ss_val / 3600; ss_val %= 3600;
	int32_t mm_val = ss_val / 60; ss_val %= 60;

	time_str[0] = '0' + hh_val / 10;
	time_str[1] = '0' + hh_val % 10;
	time_str[2] = ':';
	time_str[3] = '0' + mm_val / 10;
	time_str[4] = '0' + mm_val % 10;
	time_str[5] = ':';
	time_str[6] = '0' + ss_val / 10;
	time_str[7] = '0' + ss_val % 10;
	time_str[8] = ',';
	time_str[9] = '0' + ms_val / 100; ms_val %= 100;
	time_str[10] = '0' + ms_val / 10; ms_val %= 10;
	time_str[11] = '0' + ms_val;
}

char* time_shift(const char*time_str, char *buf, int32_t offset)
{
	int32_t ms_val = offset + conv_time_str_2_ms(time_str);

	conv_time_ms_2_str(ms_val, buf);

	return buf + 12;
}

char* time_transform(const char* time_str, char *buf, double ratio)
{
	int32_t ms_val = conv_time_str_2_ms(time_str);

	int32_t ms_val_new = (int32_t)(ms_val * ratio);

	conv_time_ms_2_str(ms_val_new, buf);

	return buf + 12;
}

void srt_transform(const char *src_path, const char *dst_path, double ratio)
{
	FILE *pfi = fopen(src_path, "rb");
	if (!pfi)
		return;
	FILE *pfo = fopen(dst_path, "w+b");
	if (!pfo)
	{
		fclose(pfi);
		return;
	}

	char buf[1024], dst_buf[1024];
	while (!feof(pfi))
	{
		fgets(buf, sizeof(buf) / sizeof(buf[0]), pfi);

		if (!strstr(buf, "-->"))
		{
			fprintf(pfo, "%s", buf);
			continue;;
		}

		char *ptr_1 = strchr(buf, ',');
		char *ptr_2 = strchr(ptr_1 + 4, ',');

		ptr_1 -= 8;
		ptr_2 -= 8;

		char *ptr = dst_buf;
		ptr = time_transform(ptr_1, ptr, ratio);

		strcpy(ptr, " --> ");
		ptr += 5;

		ptr = time_transform(ptr_2, ptr, ratio);
		ptr[0] = '\0';

		fprintf(pfo, "%s\n", dst_buf);
	}

	fclose(pfo);
	fclose(pfi);
}

void srt_shift(const char *src_path, const char *dst_path, int32_t offset)
{
	FILE *pfi = fopen(src_path, "rb");
	if (!pfi)
		return;
	FILE *pfo = fopen(dst_path, "w+b");
	if (!pfo)
	{
		fclose(pfi);
		return;
	}

	char buf[1024], dst_buf[1024];
	while (!feof(pfi))
	{
		fgets(buf, sizeof(buf) / sizeof(buf[0]), pfi);

		if (!strstr(buf, "-->"))
		{
			fprintf(pfo, "%s", buf);
			continue;;
		}

		char *ptr_1 = strchr(buf, ',');
		char *ptr_2 = strchr(ptr_1 + 4, ',');

		ptr_1 -= 8;
		ptr_2 -= 8;

		char *ptr = dst_buf;
		ptr = time_shift(ptr_1, ptr, offset);

		strcpy(ptr, " --> ");
		ptr += 5;

		ptr = time_shift(ptr_2, ptr, offset);
		ptr[0] = '\0';

		fprintf(pfo, "%s\n", dst_buf);
	}

	fclose(pfo);
	fclose(pfi);
}

int main(int argc, char* argv[])
{
	if (argc != 5)
	{
		fprintf(stdout, "%s cmd input output arg\n", argv[0]);
		fprintf(stdout, "\tcmd: move/transform\n");
		fprintf(stdout, "\targ: ratio/offset\n");
		fprintf(stdout, "\t\tratio: large/small\n");
		fprintf(stdout, "\t\toffset: integer time offset in ms\n");
		return -1;
	}

	if (!_stricmp(argv[1], "move"))
	{
		int32_t offset = atoi(argv[4]);

		srt_shift(argv[2], argv[3], offset);
	}
	else if (!_stricmp(argv[1], "transform"))
	{
		double ratio = 23.976 / 25;
		if (!_stricmp(argv[4], "large"))
			ratio = 25 / 23.976;

		srt_transform(argv[2], argv[3], ratio);
	}
	else
	{
		fprintf(stdout, "invalid cmd %s\n", argv[1]);
		return -2;
	}

	return 0;
}

