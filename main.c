#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int shouldScanFile(const char *fileName) {
  const char *extension = strrchr(fileName, '.');
  if (extension && strncmp(extension, ".razor", 6) == 0) {
    return 1;
  }
  return 0;
}

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

  pclose(result);

  return 1;
}

int checkFiles(const char *basePath, const char *icon) {
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir(basePath)))
    return 0;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);
    printf("\n%s\n", path);

    if (entry->d_type == DT_DIR) {
      if (checkFiles(path, icon)) {
        return 1;
      }
    } else if (shouldScanFile(entry->d_name) &&
               fileContainsString(path, icon)) {
      return 1;
    }
  }

  closedir(dir);

  return 0;
}

void trimIconFile(const char *iconPath, const char *trimmedIconPath,
                  const char *startDir) {
  char command[256];

  // Duplicate the icon file
  memset(command, 0, 256);
  strcpy(command, "jq . ");
  strncat(command, iconPath, strlen(iconPath));
  strncat(command, " > ", 4);
  strncat(command, trimmedIconPath, strlen(trimmedIconPath));

  printf("jq command: %s\n", command);
  int rc = system(command);
  if (rc != 0) {
    printf("command execution failed\n");
    return;
  }

  // Get icon list
  memset(command, 0, 256);
  strcpy(command, "jq .icons ");
  strncat(command, trimmedIconPath, strlen(trimmedIconPath));

  printf("jq command: %s\n", command);
  FILE *result = popen(command, "r");

  char line[256];
  while (fgets(line, 256, result)) {
    if (line[0] != '{' && line[0] != '}') {
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

      if (!checkFiles(startDir, icon)) {
        // Remove icon from the file
        memset(command, 0, 256);
        strcpy(command, "jq 'del(.icons.\"");
        strncat(command, icon, strlen(icon));
        strncat(command, "\")' ", 6);
        strncat(command, trimmedIconPath, strlen(trimmedIconPath));
        strncat(command, " | sponge ", 11);
        strncat(command, trimmedIconPath, strlen(trimmedIconPath));

        printf("%s\n", command);

        rc = system(command);
        if (rc != 0) {
          printf("command execution failed\n");
          return;
        }
      }
    }
  }

  pclose(result);
}

int main(int argc, char *argv[]) {
  char iconFilePath[256];
  char trimmedIconFilePath[256];
  char startDir[256];

  printf("Please enter the path to the icon file: ");
  scanf("%s", iconFilePath);
  printf("Please enter the path for the trimmed icon file: ");
  scanf("%s", trimmedIconFilePath);
  printf("Please enter the directory to scan: ");
  scanf("%s", startDir);

  trimIconFile(iconFilePath, trimmedIconFilePath, startDir);

  return 0;
}
