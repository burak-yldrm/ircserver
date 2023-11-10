#include "../include/Server.hpp"

Server::Server(int serverSocketFamily, int serverSocketProtocol, int serverSocketPort)
	: _serverSocketFD(-1),
	_serverSocketFamily(serverSocketFamily),
	_serverSocketProtocol(serverSocketProtocol),
	_serverSocketPort(serverSocketPort)
{
	memset(&serverAddress, 0, sizeof(serverAddress));
	memset(&serverAddress6, 0, sizeof(serverAddress6));

#if defined(__linux__)
	epollFd = epoll_create1(0);
	if (epollFd == -1) {
		// Hata işleme...
	}
#elif defined(__APPLE__) || defined(__MACH__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
	kq = kqueue();
	if (kq == -1) {
		// Hata işleme...
	}
#endif
}

Server::~Server()
{
	if (_serverSocketFD != -1)
	{
		close(_serverSocketFD);
	}
#if defined(__linux__)
	if (epollFd != -1)
	{
		close(epollFd);
	}
#elif defined(__APPLE__) || defined(__MACH__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
	if (kq != -1)
	{
		close(kq);
	}
#endif
}

void Server::socketStart()
{
	_serverSocketFD = socket( _serverSocketFamily, _serverSocketProtocol, 0 );

	if ( _serverSocketFD == -1 )
		ErrorLogger( FAILED_SOCKET, __FILE__, __LINE__ );

	if ( fcntl(_serverSocketFD, F_SETFL, O_NONBLOCK) == -1 )
	{
		close(_serverSocketFD);
		ErrorLogger( FAILED_SOCKET_NONBLOCKING, __FILE__, __LINE__ );
	}

	int reuse = 1;
	if ( setsockopt(_serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1 )
	{
		close(_serverSocketFD);
		ErrorLogger( FAILED_SOCKET_OPTIONS, __FILE__, __LINE__ );
	}

}

void Server::socketInit()
{
	switch ( _serverSocketFamily )
	{
		case AF_INET:
			serverAddress.sin_addr.s_addr = INADDR_ANY;
			serverAddress.sin_family = _serverSocketFamily;
			serverAddress.sin_port = htons( _serverSocketPort );
			break;
		case AF_INET6:
			serverAddress6.sin6_addr = in6addr_any;
			serverAddress6.sin6_family = _serverSocketFamily;
			serverAddress6.sin6_port = htons( _serverSocketPort );
			break;
		default:
			close(_serverSocketFD);
			ErrorLogger( FAILED_SOCKET_DOMAIN, __FILE__, __LINE__ );
	}
}

void Server::socketBind()
{
	switch (_serverSocketFamily)
	{
		case AF_INET: {
			if ( ::bind(_serverSocketFD, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1 )
			{
				close(_serverSocketFD);
				ErrorLogger(FAILED_SOCKET_BIND, __FILE__, __LINE__);
			}
			break;
		}
		case AF_INET6:
		{
			if ( ::bind(_serverSocketFD, reinterpret_cast<struct sockaddr *>(&serverAddress6), sizeof(serverAddress6)) == -1 )
			{
				close(_serverSocketFD);
				ErrorLogger(FAILED_SOCKET_BIND, __FILE__, __LINE__);
			}
			break;
		}
	}
}

void Server::socketListen()
{
	if ( listen(_serverSocketFD, BACKLOG_SIZE) == -1 )
	{
		close(_serverSocketFD);
		ErrorLogger(FAILED_SOCKET_LISTEN, __FILE__, __LINE__);
	}

#if defined(__linux__)
	struct epoll_event ev;
	ev.events = EPOLLIN; // Yeni bağlantılar için EPOLLIN yeterlidir.
	ev.data.fd = _serverSocketFD;  // server soket file descriptor

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _serverSocketFD, &ev) == -1)
	{
		perror("epoll_ctl: server socket");
		// exit(EXIT_FAILURE);
	}
#elif defined(__APPLE__) || defined(__MACH__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
	struct kevent evSet;
	EV_SET(&evSet, _serverSocketFD, EVFILT_READ, EV_ADD, 0, 0, NULL);

	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		perror("kevent: server socket");
		// exit(EXIT_FAILURE);
	}
#endif
}

int Server::socketAccept()
{
	struct sockaddr_storage clientAddress; // hem IPv4 hem de IPv6 adresleri için yeterince büyük
	socklen_t clientAddressLength = sizeof(clientAddress);

	int clientSocketFD = accept(_serverSocketFD, (struct sockaddr *)&clientAddress, &clientAddressLength);

	if (clientSocketFD == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{

		}
		else
		{
			ErrorLogger(FAILED_SOCKET_ACCEPT, __FILE__, __LINE__);
			return -1;
		}
	}
#if defined(__linux__)
	struct epoll_event event;
	event.data.fd = clientSocketFD;
	event.events = EPOLLIN | EPOLLET; // Edge-triggered read

	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocketFD, &event) == -1)
	{
		close(clientSocketFD);
		ErrorLogger(FAILED_SOCKET_EPOLL_CTL, __FILE__, __LINE__);
	}
#elif defined(__APPLE__) || defined(__MACH__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
	struct kevent evSet;
	EV_SET(&evSet, clientSocketFD, EVFILT_READ, EV_ADD, 0, 0, NULL);

	if (kevent(kq, &evSet, 1, NULL, 0, NULL) == -1)
	{
		close(clientSocketFD);
		ErrorLogger(FAILED_SOCKET_KQUEUE_KEVENT, __FILE__, __LINE__);
	}
#endif
	std::cout << "New client connected: " << clientSocketFD << std::endl;
	return clientSocketFD;
}

void Server::handleClient(int clientSocketFD)
{
	// Bu örnekte, basit bir buffer kullanıyoruz.
	char buffer[512];
	memset(buffer, 0, sizeof(buffer));

	ssize_t received = recv(clientSocketFD, buffer, sizeof(buffer), 0);
	if (received > 0) {
		// Veriyi aldık, işlememiz gerekiyor.
		// Örneğin, buffer'daki komutları ayrıştırabilir ve cevaplayabiliriz.
		std::cout << "Received message from client " << clientSocketFD << ": " << buffer << std::endl;

		// Gelen mesaja göre eylem yapın (mesajı yorumlayın ve uygun komutları işleyin)
	} else if (received == 0) {
		// Istemci bağlantıyı kapattı.
		std::cout << "Client " << clientSocketFD << " disconnected." << std::endl;
		close(clientSocketFD); // Soketi kapatın.
		// Bağlantıyı istemci listesinden çıkarın (eğer varsa).
	} else {
		// Hata alındı.
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			ErrorLogger("recv error", __FILE__, __LINE__);
			close(clientSocketFD); // Soketi kapatın.
			// Bağlantıyı istemci listesinden çıkarın (eğer varsa).
		}
	}
}



void Server::serverRun()
{
	socketStart();
	socketInit();
	socketBind();
	socketListen();

	// Sunucunun ana döngüsü
	while (true)
	{
		// `epoll` veya `kqueue` ile mevcut bağlantıları dinleyin ve yeni bağlantıları kabul edin.
		// Bu örnek sadece `epoll` için yazılmıştır. `kqueue` için benzer bir yaklaşım uygulanmalıdır.
#if defined(__linux__)
		struct epoll_event events[MAX_CLIENTS]; // MAX_EVENTS tanımlanmalıdır.
		int n = epoll_wait(epollFd, events, MAX_CLIENTS, -1);
		for (int i = 0; i < n; i++) {
			if (events[i].data.fd == _serverSocketFD) {
				// Yeni bir bağlantıyı kabul edin.
				int clientFD = socketAccept();
				if (clientFD != -1) {
					// Yeni bağlantıyı işleyin (örneğin, bir liste/map'e ekleyin)
				}
			} else {
				// Mevcut bir istemciden veri alındı.
				handleClient(events[i].data.fd);
			}
		}
#elif defined(__APPLE__) || defined(__MACH__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined (__NetBSD__)
		struct kevent evList[MAX_CLIENTS];
		int n = kevent(kq, NULL, 0, evList, MAX_CLIENTS, NULL);
		for (int i = 0; i < n; i++) {
			if (evList[i].ident == _serverSocketFD) {
				// Yeni bir bağlantıyı kabul edin.
				int clientFD = socketAccept();
				if (clientFD != -1) {
					// Yeni bağlantıyı işleyin (örneğin, bir liste/map'e ekleyin)
				}
			} else {
				// Mevcut bir istemciden veri alındı.
				handleClient(evList[i].ident);
			}
		}
#endif
	}
}
