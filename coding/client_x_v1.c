/* Cross-platform TLS chat client -- compiles on Linux (gcc) and
   Windows (via MinGW-w64 cross-compile, or natively with MSVC-compatible
   setup). See the #ifdef _WIN32 blocks for the platform-specific bits.

   DESIGN NOTE: the original Linux-only client used select() to watch
   both stdin and the socket at once. Winsock's select() can ONLY watch
   sockets -- it cannot watch stdin or any other non-socket handle, so
   that design doesn't port. Instead, this version uses two threads:
   one dedicated to reading your typed input and sending it, one
   dedicated to receiving and printing incoming messages. This works
   identically on both platforms and needs no select() at all. */

/* Copiling steps:
   sudo apt install mingw-w64
   git clone --depth 1 --branch openssl-3.0.13 https://github.com/openssl/openssl.git openssl-src
   cd openssl-src/
   ./Configure mingw64 --cross-compile-prefix=x86_64-w64-mingw32- no-tests
   make build_libs

# Linux bulid
   gcc -Wall -Wextra client_x.c -o client_x -lssl -lcrypto -lpthread

 # Windows, cross-compiled from Linux (what I did):
 x86_64-w64-mingw32-gcc client_x.c -o client_x.exe -I/home/user/path/to/github/openssl-src/include -L/home/user/path/to/github/openssl-src -lssl -lcrypto -lws2_32 -lgdi32 -lcrypt32 -static
 */

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSESOCKET closesocket
    typedef SOCKET sock_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <pthread.h>
    #define CLOSESOCKET close
    typedef int sock_t;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define LINEBUF_SIZE 1024

/* ---------------- conn_t / read_line: unchanged from before ---------------- */

typedef struct {
    SSL *ssl;
    char buf[LINEBUF_SIZE];
    int buf_len;
} conn_t;

void conn_init(conn_t *c, SSL *ssl) {
    c->ssl = ssl;
    c->buf_len = 0;
}

int read_line(conn_t *c, char *line_out, size_t max_len) {
    while (1) {
        char *newline_pos = memchr(c->buf, '\n', c->buf_len);
        if (newline_pos != NULL) {
            int line_len = newline_pos - c->buf;
            if ((size_t)line_len >= max_len) return -1;

            memcpy(line_out, c->buf, line_len);
            line_out[line_len] = '\0';

            int consumed = line_len + 1;
            int remaining = c->buf_len - consumed;
            memmove(c->buf, c->buf + consumed, remaining);
            c->buf_len = remaining;

            return line_len;
        }

        if (c->buf_len >= LINEBUF_SIZE) return -1;

        int n = SSL_read(c->ssl, c->buf + c->buf_len, LINEBUF_SIZE - c->buf_len);
        if (n <= 0) {
            int err = SSL_get_error(c->ssl, n);
            if (err == SSL_ERROR_ZERO_RETURN) return 0;
            return -1;
        }

        c->buf_len += n;
    }
}

/* ---------------- Config file loading: unchanged ---------------- */

typedef struct {
    char host[256];
    int port;
} Config;

int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) { perror("Error opening config file"); return 0; }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        sscanf(line, " host = %255s", config->host);
        sscanf(line, " port = %d", &config->port);
    }

    fclose(file);
    return 1;
}

/* ---------------- Sender thread: reads stdin, sends to server ---------------- */
/* This replaces the "watch stdin with select()" part of the old design. */

typedef struct {
    SSL *ssl;
    const char *name;
} sender_args_t;

#ifdef _WIN32
DWORD WINAPI sender_thread(LPVOID arg) {
#else
void *sender_thread(void *arg) {
#endif
    sender_args_t *args = (sender_args_t *)arg;
    char typed[LINEBUF_SIZE];

    while (fgets(typed, sizeof(typed), stdin) != NULL) {
        typed[strcspn(typed, "\n")] = '\0';

        char out[LINEBUF_SIZE + 64];
        snprintf(out, sizeof(out), "%s: %s\n", args->name, typed);

        if (SSL_write(args->ssl, out, (int)strlen(out)) <= 0) {
            break; // connection broken -- let this thread end
        }
    }

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

/* ---------------- Main ---------------- */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <your name>\n", argv[0]);
        exit(1);
    }

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        exit(1);
    }
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    Config config;
    strncpy(config.host, "127.0.0.1", sizeof(config.host) - 1);
    config.host[sizeof(config.host) - 1] = '\0';
    config.port = 8443;

    if (!load_config("config.cfg", &config)) {
        printf("config.cfg could not be found, using defaults.\n");
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", config.port);

    int status = getaddrinfo(config.host, port_str, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        exit(1);
    }

    sock_t sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#ifdef _WIN32
    if (sock_fd == INVALID_SOCKET) {
#else
    if (sock_fd < 0) {
#endif
        perror("socket failed");
        freeaddrinfo(res);
        exit(1);
    }

    if (connect(sock_fd, res->ai_addr, (int)res->ai_addrlen) < 0) {
        perror("connection failed");
        freeaddrinfo(res);
        exit(1);
    }
    freeaddrinfo(res);

    // --- TLS handshake (identical on both platforms) ---
    SSL_library_init();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) { ERR_print_errors_fp(stderr); exit(1); }

    if (SSL_CTX_load_verify_locations(ctx, "server.crt", NULL) <= 0) {
        ERR_print_errors_fp(stderr); exit(1);
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, (int)sock_fd);

    if (SSL_connect(ssl) <= 0) {
        fprintf(stderr, "TLS handshake failed:\n");
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    printf("Connected and TLS handshake OK as %s. Type a message and press Enter.\n", argv[1]);

    // --- Start the sender thread, then use THIS thread as the receiver ---
    sender_args_t args = { ssl, argv[1] };

#ifdef _WIN32
    HANDLE thread = CreateThread(NULL, 0, sender_thread, &args, 0, NULL);
    if (thread == NULL) {
        fprintf(stderr, "Failed to create sender thread\n");
        exit(1);
    }
#else
    pthread_t thread;
    if (pthread_create(&thread, NULL, sender_thread, &args) != 0) {
        perror("Failed to create sender thread");
        exit(1);
    }
    pthread_detach(thread); // we won't join it -- process exit cleans it up
#endif

    conn_t conn;
    conn_init(&conn, ssl);

    char line[LINEBUF_SIZE];
    int len;
    while ((len = read_line(&conn, line, sizeof(line))) > 0) {
        printf("%s\n", line);
    }

    if (len == 0) {
        printf("Server closed the connection.\n");
    } else {
        printf("Error reading from server.\n");
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    CLOSESOCKET(sock_fd);
    SSL_CTX_free(ctx);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
