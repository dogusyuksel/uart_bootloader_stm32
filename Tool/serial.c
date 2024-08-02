// socat -d -d pty,raw,echo=0 pty,raw,echo=0

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>

#define VERSION "1.0.0."
#define TIMEOUT_MS 5000

sem_t mutex;

static int set_interface_attribs(int fd, int speed) {
    struct termios tty;
    (void)speed;
    memset(&tty, 0, sizeof(tty));

    /* get current attributes */
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Serial tcgetattr error\n");
        return 1;
    }

    /* set data bits to 8, parity to none, stop bits to 1  */
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CSIZE;

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    cfmakeraw(&tty);

    /* set attributes */
    if (tcsetattr(fd, TCSANOW, &tty)) {
        fprintf(stderr, "Serial tcsetattr error\n");
        return 1;
    }

    return 0;
}

static void *message_listener_task2(void *vargp) {
    int rc;
    char buffer[5000];
    struct pollfd fds[1];
    unsigned char byte = 0;
    int fd = -1;

    if (!vargp) {
        goto out;
    }

    fd = *((int *)vargp);

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    while (1) {
        rc = poll(fds, 1, TIMEOUT_MS);
        if (rc < 0) {
            fprintf(stderr, "poll() failed");
            goto out;
        }

        if (fds[0].revents != POLLIN) {
            continue;
        }

        memset(buffer, 0, sizeof(buffer));

        while (read(fds[0].fd, &byte, sizeof(unsigned char)) == 1) {
            if (write(fd, &byte, 1) == -1) {
                fprintf(stderr, "write() failed");
            }
        }

        sem_wait(&mutex);
    }

out:
    pthread_exit(NULL);
}

static void *message_listener_task1(void *vargp) {
    int rc;
    char buffer[5000];
    struct pollfd fds[1];
    unsigned char byte = 0;
    int fd = -1;

    if (!vargp) {
        goto out;
    }

    fd = *((int *)vargp);

    fds[0].fd = fd;
    fds[0].events = POLLIN; // poll input events

    while (1) {
        rc = poll(fds, 1, TIMEOUT_MS);
        if (rc < 0) {
            fprintf(stderr, "poll() failed");
            goto out;
        }

        // go throught the current list and skip if there is no POLLIN
        if (fds[0].revents != POLLIN) {
            continue;
        }

        memset(buffer, 0, sizeof(buffer));

        while (read(fds[0].fd, &byte, sizeof(unsigned char)) == 1) {
            if (write(STDOUT_FILENO, &byte, 1) == -1) {
                fprintf(stderr, "write() failed");
            }
        }

        sem_post(&mutex);
    }

out:
    pthread_exit(NULL);
}

int main(int argn, char **args) {
    char *portname = NULL;
    pthread_t tid1;
    pthread_t tid2;
    int fd = -1;

    if (argn != 2) {
        fprintf(stderr, "give port name as argument\n");
        return 1;
    }
    fprintf(stderr, "%s version: %s\n", args[0], VERSION);

    portname = args[1];

    fd = open(portname, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "port cannot opened\n");
        return 1;
    }

    set_interface_attribs(fd, B115200);

    pthread_create(&tid2, NULL, message_listener_task2, &fd);
    sleep(2); // be sure message_listener_task2() is executed first
    pthread_create(&tid1, NULL, message_listener_task1, &fd);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    close(fd);

    return 0;
}
