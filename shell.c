/*
* 간단한 쉘 프로그램
* 파일 이름 : shell.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>

#define MAX_BUF 1024
#define MAX_ARG 100

// 함수 선언
void Sigint_Handler(int signo);
void Sigquit_Handler(int signo);
int getargs(char *cmd, char **argv);
void handle_redirection_and_pipe(char **argv);
void execute_command(char **argv, int background);
void handle_cd(char **argv);
void handle_pwd();
void handle_ls(char **argv);
void handle_mkdir(char **argv);
void handle_rmdir(char **argv);
void handle_ln(char **argv);
void handle_cp(char **argv);
void handle_rm(char **argv);
void handle_mv(char **argv);
void handle_cat(char **argv);

void Sigint_Handler(int signo) {
    printf("\nCtrl+C 가 정상적으로 처리되었습니다.\n");
}

void Sigquit_Handler(int signo) {
    printf("\nCtrl+Z 가 정상적으로 처리되었습니다.\n");
}

// 명령어를 공백 기준으로 분리하는 함수
int getargs(char *cmd, char **argv) {
    int narg = 0;
    while (*cmd) {
        if (*cmd == ' ' || *cmd == '\t')
            *cmd++ = '\0';
        else {
            argv[narg++] = cmd++;
            while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
                cmd++;
        }
    }
    argv[narg] = NULL;
    return narg;
}

// 파일 재지향 및 파이프 처리 함수
void handle_redirection_and_pipe(char **argv) {
    int i;
    int fd_in = -1, fd_out = -1;
    int pipefd[2];
    pid_t pid;

    // Handle input redirection (<)
    for (i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "<") == 0) {
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "No input file specified\n");
                return;
            }
            fd_in = open(argv[i + 1], O_RDONLY);
            if (fd_in == -1) {
                perror("Input redirection failed");
                return;
            }
            argv[i] = NULL; // Remove the "<" and the file name from argv
            break;
        }
    }

    // Handle output redirection (>)
    for (i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], ">") == 0) {
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "No output file specified\n");
                return;
            }
            fd_out = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out == -1) {
                perror("Output redirection failed");
                return;
            }
            argv[i] = NULL; // Remove the ">" and the file name from argv
            break;
        }
    }

    // Handle piping (|)
    for (i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "|") == 0) {
            argv[i] = NULL; // Split the command at the pipe
            pipe(pipefd);
            pid = fork();
            if (pid == 0) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO); // Redirect output to pipe
                execvp(argv[0], argv);
                perror("execvp failed");
                exit(1);
            } else {
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO); // Redirect input from pipe
                execvp(argv[i + 1], &argv[i + 1]);
                perror("execvp failed");
                exit(1);
            }
        }
    }

    // Handle redirection to/from files
    if (fd_in != -1) {
        dup2(fd_in, STDIN_FILENO); // Redirect input from file
        close(fd_in);
    }
    if (fd_out != -1) {
        dup2(fd_out, STDOUT_FILENO); // Redirect output to file
        close(fd_out);
    }

    // Execute the command normally if no redirection or pipe
    execvp(argv[0], argv);
    perror("execvp failed");
}

// 명령어 실행 함수
void execute_command(char **argv, int background) {
    pid_t pid;

    // cd 명령어는 시스템 호출을 통해 처리
    if (strcmp(argv[0], "cd") == 0) {
        handle_cd(argv);
        return;
    }

    // pwd 명령어
    if (strcmp(argv[0], "pwd") == 0) {
        handle_pwd();
        return;
    }

    // ls 명령어
    if (strcmp(argv[0], "ls") == 0) {
        handle_ls(argv);
        return;
    }

    // mkdir 명령어
    if (strcmp(argv[0], "mkdir") == 0) {
        handle_mkdir(argv);
        return;
    }

    // rmdir 명령어
    if (strcmp(argv[0], "rmdir") == 0) {
        handle_rmdir(argv);
        return;
    }

    // ln 명령어
    if (strcmp(argv[0], "ln") == 0) {
        handle_ln(argv);
        return;
    }

    // cp 명령어
    if (strcmp(argv[0], "cp") == 0) {
        handle_cp(argv);
        return;
    }

    // rm 명령어
    if (strcmp(argv[0], "rm") == 0) {
        handle_rm(argv);
        return;
    }

    // mv 명령어
    if (strcmp(argv[0], "mv") == 0) {
        handle_mv(argv);
        return;
    }

    // cat 명령어
    if (strcmp(argv[0], "cat") == 0) {
        handle_cat(argv);
        return;
    }

    pid = fork();
    if (pid == 0) {
        // 자식 프로세스에서 명령어 실행
        handle_redirection_and_pipe(argv);
        execvp(argv[0], argv);
        perror("execvp 실패");
        exit(1);
    } else if (pid > 0) {
        if (!background) {
            wait(NULL); // 백그라운드가 아니면 자식 프로세스가 종료될 때까지 대기
        }
    } else {
        perror("fork 실패");
    }
}

// cd 명령어 처리
void handle_cd(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "디렉토리를 지정해야 합니다.\n");
    } else {
        if (chdir(argv[1]) == -1) {
            perror("cd 실패");
        }
    }
}

// pwd 명령어 처리
void handle_pwd() {
    char cwd[MAX_BUF];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd 실패");
    }
}

// ls 명령어 처리
void handle_ls(char **argv) {
    DIR *dir;
    struct dirent *entry;

    if (argv[1] == NULL) {
        dir = opendir(".");
    } else {
        dir = opendir(argv[1]);
    }

    if (dir == NULL) {
        perror("ls 실패");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

// mkdir 명령어 처리
void handle_mkdir(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "디렉토리 이름을 지정해야 합니다.\n");
    } else {
        if (mkdir(argv[1], 0755) == -1) {
            perror("mkdir 실패");
        }
    }
}

// rmdir 명령어 처리
void handle_rmdir(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "디렉토리 이름을 지정해야 합니다.\n");
    } else {
        if (rmdir(argv[1]) == -1) {
            perror("rmdir 실패");
        }
    }
}

// ln 명령어 처리 (하드링크 생성)
void handle_ln(char **argv) {
    if (argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "링크할 파일과 이름을 지정해야 합니다.\n");
    } else {
        if (link(argv[1], argv[2]) == -1) {
            perror("ln 실패");
        }
    }
}

// cp 명령어 처리
void handle_cp(char **argv) {
    if (argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "복사할 파일과 대상 파일을 지정해야 합니다.\n");
    } else {
        FILE *src = fopen(argv[1], "rb");
        if (src == NULL) {
            perror("소스 파일 열기 실패");
            return;
        }
        FILE *dest = fopen(argv[2], "wb");
        if (dest == NULL) {
            perror("대상 파일 열기 실패");
            fclose(src);
            return;
        }
        
        char buffer[1024];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, n, dest);
        }

        fclose(src);
        fclose(dest);
    }
}

// rm 명령어 처리
void handle_rm(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "삭제할 파일을 지정해야 합니다.\n");
    } else {
        if (remove(argv[1]) == -1) {
            perror("rm 실패");
        }
    }
}

// mv 명령어 처리
void handle_mv(char **argv) {
    if (argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "이동할 파일과 대상 파일을 지정해야 합니다.\n");
    } else {
        if (rename(argv[1], argv[2]) == -1) {
            perror("mv 실패");
        }
    }
}

// cat 명령어 처리
void handle_cat(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "파일 이름을 지정해야 합니다.\n");
        return;
    }

    // 파일 열기 (쓰기 모드)
    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("파일 열기 실패");
        return;
    }

    char buffer[MAX_BUF];
    ssize_t bytesRead;

    // 표준 입력에서 읽어들여 파일에 쓰기
    while ((bytesRead = read(STDIN_FILENO, buffer, MAX_BUF)) > 0) {
        if (write(fd, buffer, bytesRead) == -1) {
            perror("파일 쓰기 실패");
            close(fd);
            return;
        }
    }

    close(fd);
    printf("파일에 내용이 성공적으로 기록되었습니다.\n");
}

int main() {
    char buf[MAX_BUF];
    char *argv[MAX_ARG];
    int narg;
    pid_t pid;

    // Set up signal handlers
    signal(SIGINT, Sigint_Handler);
    signal(SIGQUIT, Sigquit_Handler);

    while (1) {
        printf("shell> ");
        if (fgets(buf, MAX_BUF, stdin) == NULL) {
            printf("\n종료합니다.\n");
            break;
        }

        buf[strcspn(buf, "\n")] = '\0'; // Remove the newline character
        if (strcmp(buf, "exit") == 0) {
            printf("쉘을 종료합니다.\n");
            break;
        }

        narg = getargs(buf, argv);
        if (narg == 0) continue;

        pid = fork();

        if (pid == 0) {
            handle_redirection_and_pipe(argv);
            exit(0);
        } else if (pid > 0) {
            wait(NULL); // Wait for child process to finish
        } else {
            perror("fork failed");
        }
    }
    return 0;
}
