#include "bbcp.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct stat stat_t;

int open_target(const char *, const char *);
int filename_pos(const char *);

int main(int argc, char const *argv[]) {
  const char *program_name = argv[0];
  const char *usr_src_path;
  const char *usr_dest_path;
  char buffer[BUFFER_SIZE];
  stat_t stat_buff;
  int return_flag = 0;
  int src_fd;
  int dest_fd;
  int read_num;
  int written_num;

  /* check arg number */
  if (argc != 3) {
    printf("Usage: %s source destination\n",
           program_name + filename_pos(argv[0]));
    return WRONG_NUMBER_ARGS;
  }

  usr_src_path = argv[1];
  usr_dest_path = argv[2];

  /* check source */
  if (stat(usr_src_path, &stat_buff) == -1) {
    perror(SRC_ER_MSG);
    return INVALID_SOURCE;
  }

  if (!S_ISREG(stat_buff.st_mode)) {
    printf(SRC_ER_MSG);
    printf("\n");
    return INVALID_SOURCE;
  }

  /* open source */
  if ((src_fd = open(usr_src_path, O_RDONLY)) != -1) {
    /* open target */
    if ((dest_fd = open_target(usr_src_path, usr_dest_path)) != -1) {
      /* buffered write loop */
      while ((read_num = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        written_num = write(dest_fd, buffer, read_num);

        if (written_num == -1) {
          perror(CPY_ER_MSG);
          return_flag = WRITE_ERROR;
          break;
        }  // single retry attempt
        else if (written_num != read_num) {
          written_num = write(dest_fd, buffer, BUFFER_SIZE);
          if (written_num != read_num) {
            printf(CPY_ER_MSG);
            printf("\n");
            return_flag = WRITE_ERROR;
            break;
          }
        }
      }
      if (read_num == -1) {
        perror(CPY_ER_MSG);
        return_flag = WRITE_ERROR;
      }

      close(dest_fd);
    }

    close(src_fd);
    return return_flag;
  }

  perror(SRC_ER_MSG);
  return INVALID_SOURCE;
}

int open_target(const char *usr_src_path, const char *usr_dest_path) {
  const char *src_filename;
  char *dest_path;
  long usr_dest_path_len;
  long src_filename_len;
  int fd;
  stat_t stat_buff;
  int pos;
  char *str_end_addr;
  mode_t permissions;

  if (stat(usr_src_path, &stat_buff) == -1) {
    perror(SRC_ER_MSG);
    return -1;
  }
  permissions = stat_buff.st_mode;

  pos = filename_pos(usr_src_path);
  if (pos == 0) {
    src_filename = usr_src_path;
  } else {
    src_filename = usr_src_path + pos;
  }
  src_filename_len = strlen(src_filename);
  usr_dest_path_len = strlen(usr_dest_path);

  pos = filename_pos(usr_dest_path);
  if (pos == 0) { /* presented destination does not conatin slashes */
    if (stat(usr_dest_path, &stat_buff) ==
        -1) { /* presented destination does not exist - it is a file name */
      if (errno != ENOENT) {
        perror(DEST_ER_MSG);
        return -1;
      }

      if ((fd = open(usr_dest_path, O_WRONLY | O_CREAT, permissions)) == -1) {
        perror(DEST_ER_MSG);
      }

      return fd;
    } else { /* presented destination exists */
      if (S_ISREG(stat_buff
                      .st_mode)) { /* presented destination is a regular file */
        if ((fd = open(usr_dest_path, O_WRONLY)) == -1) {
          perror(DEST_ER_MSG);
          return -1;
        }

        return fd;
      } else if (S_ISDIR(
                     stat_buff.st_mode)) { /* presented destination is a dir */
        dest_path = malloc(usr_dest_path_len + 1 + src_filename_len +
                           1); /* dest_dir_path + / + src_filename + \0 */
        str_end_addr = dest_path;
        memcpy(str_end_addr, usr_dest_path, usr_dest_path_len);
        str_end_addr += usr_dest_path_len;
        *str_end_addr = '/';
        str_end_addr += 1;
        memcpy(str_end_addr, src_filename, src_filename_len);

        if ((fd = open(dest_path, O_WRONLY | O_CREAT, permissions)) == -1) {
          perror(DEST_ER_MSG);
        }

        free(dest_path);
        return fd;
      } else { /* presented destination is invalid */
        printf(DEST_ER_MSG);
        printf("\n");
        return -1;
      }
    }
  } else { /* there is a slash in the middle of destination path */
    if ((stat(usr_dest_path, &stat_buff)) == -1) {
      if (errno != ENOENT) {
        perror(DEST_ER_MSG);
        return -1;
      }
      /* final part of the path does not exist */

      if ((fd = open(usr_dest_path, O_WRONLY | O_CREAT, permissions)) == -1) {
        perror(DEST_ER_MSG);
      }
      return fd;
    }

    if (S_ISDIR(stat_buff.st_mode)) { /* final part of the path is a dir */
      dest_path = malloc(usr_dest_path_len + 1 + src_filename_len + 1);
      str_end_addr = dest_path;
      memcpy(str_end_addr, usr_dest_path, usr_dest_path_len);
      str_end_addr += usr_dest_path_len;
      if (*(str_end_addr - 1) != '/') {
        *str_end_addr = '/';
        str_end_addr += 1;
      }
      memcpy(str_end_addr, src_filename, src_filename_len);

      if ((fd = open(dest_path, O_WRONLY | O_CREAT, permissions)) == -1) {
        perror(DEST_ER_MSG);
      }

      free(dest_path);
      return fd;
    } else if (S_ISREG(
                   stat_buff.st_mode)) { /* final part of the path is a file */
      if ((fd = open(usr_dest_path, O_WRONLY)) == -1) {
        perror(DEST_ER_MSG);
      }
      return fd;
    } else { /* final part of the path is invalid type */
      printf(DEST_ER_MSG);
      printf("\n");
      return -1;
    }
  }

  return -1;
}

/*
 * returns position of the last char that is not a '/'
 * in the case where the are no '/' in the path, returns 0
 */
int filename_pos(const char *path) {
  int i = strlen(path) - 1; /* skip null byte */
  while (i >= 0 && path[i] != '/') {
    i--;
  }

  return i + 1;
}
