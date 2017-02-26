#socket(...)

Создает сокет и возвращает дескриптор сокета.

int socket(int domain, int type, int protocol);<br />
Домен (domain) может быть следующим:<br />
AF_INET - для IPv4,<br />
AF_INET6 - для IPv6,<br />
AF_UNIX - для Unix-сокетов ( https://en.wikipedia.org/wiki/Unix_domain_socket ).<br />
Тип (type) может быть следующий:<br />
SOCK_STREAM - для TCP,<br />
SOCK_DGRAM - для UDP.<br />
Протокол (proto) приравниваем к нулю - автоматический выбор.<br />G

#bind(...)

Осуществляет "привязку" IP-адреса и порта к серверному сокету (в том числе дейтаграмному).<br />
bind(int sockfd, struct sockaddr *addr, socklet_t addrlen);<br />

Первый параметр - дескриптор сокета.<br />
Второй параметр. Тут есть варианты:<br />
IPv4. Пройдемся по порядку.<br />
Структура называется sockaddr_in. "_in" тут означает INET (вспомните AF_INET!).<br />
#sockaddr_in sa;

Первым делом мы указываем "домен" - AF_INET.<br />
Это необходимо сделать для того, чтобы было понимание, к какому типу впоследствии приводить sockaddr.<br />
sa.sin_family = AF_INET;<br />

Далее, мы указываем порт. Указывать его нужно в виде числа в сетевом порядке байт.
Для того, чтобы этот порядок байт получить, используем функцию htons (для 16-битного числа, коим и является порт. А для 32-битного числа, например, для адреса - htonl).<br />
sa.sin_port = htons(12345);<br />

И, наконец, указываем адрес, к которому привязываем сокет. А тут есть несколько вариантов.<br />
Мы можем использовать константы:<br />
sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1<br />
sa.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0<br />
Последняя константа позволяет привязать сокет на все сетевые интерфейсы сервера.<br />
Мы можем использовать устаревшую функцию inet_addr (только для AF_INET):<br />
sa.sin_addr.s_addr = inet_addr("10.0.0.1");<br />
Мы можем использовать современную функцию inet_pton (AF_INET и AF_INET6):<br />
inet_pton(AF_INET, "10.0.0.1", &sa);<br />

#struct sockaddr_un sa;
Unix. Тут все проще простого.<br />
sa.sun_family = AF_UNIX;<br />
strcpy(sa.sun_path, "/tmp/demo.sock"); // Путь до сокета.<br />

Третий параметр - размер структуры, которая был передана вторым параметром.<br />
Возвращает - 0 при успехе и -1 при ошибке.<br />

#listen(...)

Переводит сокет в режим приема новых соединений (TCP). Параметра два:<br />

int listen(int sockfd /* Дескриптор сокета */, int backlog);<br />
Параметр backlog - максимальное длина очереди ожидающих соединения. Предельное значение - SOMAXCONN.<br />
Возвращает - 0 при успехе и -1 при ошибке.<br />

#accept(...)

Принимает новое соединение и возвращает дескриптор на него.<br />
int accept(int sockfd /* Слушающий сокет */, struct sockaddr *addr, socklen_t *addrlen);<br />

Тут важно акцентировать внимание на параметрах addr и addrlen.
Они могут быть NULL и NULL, а могут быть указателями, и тогда по addr станет доступна структура (например, sockaddr_in),
описывающая клиента (в т.ч. его IP-адрес), а по addrlen будет доступна длина этой структуры.<br />

#connect(...)

Противоположность accept. <br />
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);<br />
 
Вызывается на клиентском сокете, запускает процесс соединения с сервером.
Адрес сервера передается через addr и addrlen.<br />
#send(...)/recv(...)

ssize_t send(int sockfd, const void *buf, size_t len, int flags);<br />
ssize_t recv(int sockfd, void *buf, size_t len, int flags);<br />
 
Запись в сокет и чтение из сокета.
Главное отличие от read/write (которые, кстати, тоже допустимы) - наличие флагов flags.
Мы обычно заинтересованы в флаге MSG_NOSIGNAL,
который подавляет сигнал SIGPIPE при попытке работы с разорванным соединением.<br />
#shutdown(...)

Разрываем соединение - на чтение или/и на запись.<br />
int shutdown(int sockfd, int how /* SHUT_RD, SHUT_WR, SHUT_RDWR */);﻿<br />
 
#sendto(...)/recvfrom(...)

Это версии send/recv для UPD. Они содержат параметры ﻿addr и addrlen, необходимые для UDP (адрес получателя или отправителя).<br />
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, ﻿const struct sockaddr *dest_addr, socklen_t addrlen);<br />
﻿ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, ﻿struct sockaddr *src_addr, socklen_t *addrlen);<br />
 

#﻿Перевод сокетов в неблокирующий режим

int set_nonblock(int fd)<br />
{<br />
    ﻿int flags;<br />
    #if defined(O_NONBLOCK)<br />
    ﻿if (-1 == (flags = fcntl(fd, F_GETFL, 0)))<br />
        ﻿flags = 0;<br />
    ﻿return fcntl(fd, F_SETFL, flags | O_NONBLOCK);<br />
    #else<br />
    ﻿flags = 1;<br />
    ﻿return ioctl(fd, FIOBIO, &flags);<br />
    #endif<br />
}<br />
