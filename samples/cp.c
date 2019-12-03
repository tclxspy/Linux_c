
#include <stdio.h>
#include <string.h>

#define BUF_SIZE				255


int copy_file(const char * src_path, const char * dst_path)
{
	char buf[BUF_SIZE];
	int len_read, len_write;
	FILE * fsrc, *fdst;

	if ((fsrc = fopen(src_path, "r")) == NULL) {
		fprintf(stderr, "file : %s can't be opened.\n", src_path);
		return - 1;
	}

	if ((fdst = fopen(dst_path, "w")) == NULL) {
		fprintf(stderr, "file : %s can't be opened.\n", dst_path);
		return - 1;
	}

	memset(buf, 0, BUF_SIZE);

//	while ((len_read = fread(buf, 1, BUF_SIZE, fsrc)) > 0) {
//		if ((len_write = fwrite(buf, 1, len_read, fdst)) != len_read) {
//			fprintf(stderr, "error to write.\n");
//			fclose(fsrc);
//			fclose(fdst);
//			return - 1;
//		}
//
//		memset(buf, 0, BUF_SIZE);
//	}

	while (1) {
		len_read = fread(buf, 1, BUFSIZ, fsrc);
		if (len_read > 0) {
			len_write = fwrite(buf, 1, len_read, fdst);

			if (len_read != len_write) {
				fprintf(stderr, "error to write.\n");
				fclose(fsrc);
				fclose(fdst);
				return - 1;
			}
		}
		else 
			break;

	}

	fclose(fsrc);
	fclose(fdst);
	return 0;
}


int main(int argc, char * argv[])
{
	if (argc != 3) {
		fprintf(stderr, "argv error.\n");
		return - 1;
	}
	else {
		return copy_file(argv[1], argv[2]);
	}

	return 0;
}


