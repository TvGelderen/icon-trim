/* We want POSIX.1-2008 + XSI, i.e. SuSv4, features */
#define _XOPEN_SOURCE 700

/* Added on 2017-06-25:
   If the C library can support 64-bit file sizes
   and offsets, using the standard names,
   these defines tell the C library to do so. */
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* POSIX.1 says each process has at least 20 file descriptors.
 * Three of those belong to the standard streams.
 * Here, we use a conservative estimate of 15 available;
 * assuming we use at most two for other uses in this program,
 * we should never run into any problems.
 * Most trees are shallower than that, so it is efficient.
 * Deeper trees are traversed fine, just a bit slower.
 * (Linux allows typically hundreds to thousands of open files,
 *  so you'll probably never see any issues even if you used
 *  a much higher value, say a couple of hundred, but
 *  15 is a safe, reasonable value.)
 */
#ifndef USE_FDS
#define USE_FDS 15
#endif

int fileContainsString(const char *filepath, const char *icon) {
  char command[256];
  memset(command, 0, 256);
  strncat(command, "grep ", 6);
  strncat(command, icon, strlen(icon));
  strncat(command, " ", 2);
  strncat(command, filepath, strlen(filepath));

  printf("file:\t\t%s\n", filepath);
  printf("command:\t%s\n", command);

  FILE *result = popen(command, "r");
  if (result == NULL) {
    return errno;
  }

  char output[256];
  if (!fgets(output, 256, result)) {
    return 0;
  }

  return 1;
}

int handleFile(const char *filepath, const struct stat *info,
               const int typeflag, struct FTW *pathinfo) {
  switch (typeflag) {
  case FTW_F:
    printf(" %s\n", filepath);
    // TODO: Get icon names from input file
    if (fileContainsString(filepath, "icon-name")) {
      printf("file contains string\n\n");
    }
    break;
  case FTW_D:
  case FTW_DP:
    printf(" %s/\n", filepath);
    break;
  default:
    printf(" %s (unknown)\n", filepath);
    break;
  }

  return 0;
}

int traverseFiles(const char *const dirpath) {
  int result;

  if (dirpath == NULL || *dirpath == '\0') {
    return errno = EINVAL;
  }

  result = nftw(dirpath, handleFile, USE_FDS, FTW_PHYS);
  if (result >= 0) {
    errno = result;
  }

  return errno;
}

int main(int argc, char *argv[]) {
  char baseDir[256];

  printf("Please enter the directory containing the code to scan: ");
  scanf("%s", baseDir);

  if (traverseFiles(baseDir)) {
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
