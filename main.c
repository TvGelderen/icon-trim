#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

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
    return 0;
  }

  char output[256];
  if (!fgets(output, 256, result)) {
    return 0;
  }

  return 1;
}

void listdir(const char *name) {
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(name)))
    return;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
    printf("%s\n", path);

    if (entry->d_type == DT_DIR) {
      listdir(path);
    } else {
      if (fileContainsString(path, "icon-name")) {
        printf("file contains string\n\n");
      } else {
        printf("file does not contain string\n\n");
      }
    }
  }
  closedir(dir);
}

void getIconNames(const char *filePath) {
  char command[256];
  memset(command, 0, 256);
  strcpy(command, "jq '.icons | keys' ");
  strncat(command, filePath, strlen(filePath));

  printf("jq command: %s\n\n", command);

  FILE *result = popen(command, "r");

  char line[256];
  while (fgets(line, 256, result)) {
    if (line[0] != '[' && line[0] != ']') {
      char icon[256];
      memset(icon, 0, 256);
      int idx = 0;
      int iconLenth = 0;

      while (!isalpha(line[idx])) {
        idx++;
      }

      for (; idx < strlen(line); idx++) {
        if (line[idx] == '\"') {
          break;
        }
        icon[iconLenth++] = line[idx];
      }

      printf("%s\n", icon);
    }
  }
}

int main(int argc, char *argv[]) {
  char iconFilePath[256];
  char startDir[256];

  printf("Please enter the path to the icon file: ");
  scanf("%s", iconFilePath);

  getIconNames(iconFilePath);

  printf("Please enter the directory to scan: ");
  scanf("%s", startDir);

  listdir(startDir);

  return 0;
}
