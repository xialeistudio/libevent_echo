#include <stdio.h>
#include <evutil.h>
#include <event.h>
#include <unistd.h>
#include <string.h>

int tcp_server_init(int, int);

void handle_accept(int listenfd, short events, void *arg);

void handle_read_client(int fd, short events, void *arg);

int main() {
    printf("server starting. pid=%d\n", getpid());
    int listenfd = tcp_server_init(12000, 128);
    if (listenfd == -1) {
        return -1;
    }
    struct event_base *base = event_base_new();
    // 添加客户端请求链接事件
    struct event *ev_listen = event_new(base, listenfd, EV_READ | EV_PERSIST, handle_accept, base);
    event_add(ev_listen, NULL);
    event_base_dispatch(base);

    return 0;
}

/**
 * 初始化TCP服务器
 * @param port
 * @param backlog
 * @return
 */
int tcp_server_init(int port, int backlog) {
    evutil_socket_t listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "fail to create socket");
        return -1;
    }


    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons (port);

    if (bind(listenfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        fprintf(stderr, "fail to bind");
        evutil_closesocket(listenfd);
        return -1;
    }
    if (listen(listenfd, backlog) < 0) {
        fprintf(stderr, "fail to listen");
        evutil_closesocket(listenfd);
        return -1;
    }

    evutil_make_socket_nonblocking(listenfd);
    return listenfd;
}

/**
 * 新客户端链接
 * @param listenfd
 * @param events
 * @param arg
 */
void handle_accept(int listenfd, short events, void *arg) {
    evutil_socket_t sockfd;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    sockfd = accept(listenfd, (struct sockaddr *) &client, &len);
    evutil_make_socket_nonblocking(sockfd);
    printf("accept a client %d\n", sockfd);

    struct event_base *base = (struct event_base *) arg;

    struct event *ev = event_new(NULL, -1, 0, NULL, NULL);
    event_assign(ev, base, sockfd, EV_READ | EV_PERSIST, handle_read_client, ev);
    event_add(ev, NULL);
}

/**
 * 读取客户端数据
 * @param fd
 * @param events
 * @param arg
 */
void handle_read_client(int fd, short events, void *arg) {
    char msg[4096];
    struct event *ev = (struct event *) arg;
    ssize_t len = read(fd, msg, sizeof(msg) - 1);
    if (len <= 0) {
        fprintf(stderr, "handle_read_client error\n");
        event_free(ev);
        close(fd);
        return;
    }

    msg[len] = '\0';
    printf("recv the client msg: %s\n", msg);

    char *reply_msg = "server msg";
    write(fd, reply_msg, strlen(reply_msg));
}