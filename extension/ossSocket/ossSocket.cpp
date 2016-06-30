/*
	Copyright (C) 2016 CHAN Gonghao.
	
	This Program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License, version 3,
	as published by the Free Software Foundation.

	This Program is distributed in the hope that is will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTSBILITY or FITNESS FOR A PARTICULAR PUPOSE. See the 
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU AFFERO General Public License
	along with this program. If not, see <http://www.gnu.org/license>.
*/
#include "ossSocket.hpp"

// Create a listening socket
_ossSocket::_ossSocket ( unsigned int port, int timeout ) {
	_init = false ;
	_fd = 0 ;
	_timeout = timeout ;
	memset ( &_sockAddress, 0, sizeof(sockaddr_in) ) ;
	memset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
	_peerAddressLen = sizeof(_peerAddress) ;
	_sockAddress.sin_family = AF_INET ;
	_sockAddress.sin_addr.s_addr = htonl( INADDR_ANY ) ;
	_sockAddress.sin_port = htons ( port ) ;
	_addressLen = sizeof( _sockAddress ) ;
}

// Create a new socket
_ossSocket::_ossSocket () {
	_init = false ;
	_fd = 0 ;
	_timeout = 0;
	memset ( &_sockAddress, 0, sizeof(sockaddr_in) ) ;
	memset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
	_peerAddressLen = sizeof( _peerAddress ) ;
	_addressLen = sizeof( _sockAddress ) ;
}

// Create a connection socket
_ossSocket::_ossSocket ( const char *pHostname, unsigned int port, int timeout) {
	struct hostent *hp ;
	_init = false ;
	_timeout = timeout ;
	_fd = 0 ;
	memset ( &_sockAddress, 0, sizeof(sockaddr_in) ) ;
	memset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
	_peerAddressLen = sizeof( _peerAddress ) ;
	_sockAddress.sin_family = AF_INET ;
	if ( ( hp = gethostbyname ( pHostname ) ) )
		_sockAddress.sin_addr.s_addr = *( (int*)hp->h_addr_list[0] ) ;
	else
		_sockAddress.sin_addr.s_addr = inet_addr ( pHostname ) ;
	_sockAddress.sin_port = htons ( port ) ;
	_addressLen = sizeof( _sockAddress ) ;
}

// Create from a existing socket
_ossSocket::_ossSocket (int *sock, int timeout ) {
	int rc = GHDB_OK ;
	_fd = *sock ;
	_init = true ;
	_timeout = timeout ;
	_addressLen = sizeof( _sockAddress ) ;
	memset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
	_peerAddressLen = sizeof(_peerAddress) ;
	rc = getsockname ( _fd, (sockaddr*)&_sockAddress, &_addressLen ) ;
	if ( rc ) {
		printf("Failed to get sock name, error = %d", SOCKET_GETLASTERROR ) ;
		_init = false ;
	}
	else {
		rc = getpeername ( _fd, (sockaddr*)&_peerAddress, &_peerAddressLen ) ;
		if ( rc ) {
			printf("Failed to get peer name, error = %d", SOCKET_GETLASTERROR ) ;
		}
	}
}

int _ossSocket::initSocket () {
	int rc = GHDB_OK ;
	if ( _init ) {
		goto done ;
	}
	memset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
	_peerAddressLen = sizeof(_peerAddress) ;
	_fd = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
	if ( -1 == _fd ) {
		printf("Failed to initialize socket, error = %d", SOCKET_GETLASTERROR ) ;
		rc = GHDB_NETWORK;
		goto error;
	}
	_init = true ;
	// set timeout
	setTimeout ( _timeout ) ;
	done :
		return rc ; 
	error :
		goto done ;
}

int _ossSocket::setSocketLi ( int lOnOff, int linger ) {
	int rc = GHDB_OK ;
	struct linger _linger ;
	_linger.l_onoff = lOnOff ;
	_linger.l_linger = linger ; 
	rc = setsockopt ( _fd, SOL_SOCKET, SO_LINGER, ( const char*)&_linger, sizeof(_linger) ) ;
	return rc;
}

void _ossSocket::setAddress ( const char *pHostname, unsigned int port ) {
	struct hostent *hp ;
	memset ( &_sockAddress, 0, sizeof(sockaddr_in) ) ;
	memset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
	_peerAddressLen = sizeof(_peerAddress) ;
	_sockAddress.sin_family = AF_INET ;
	if ( ( hp = gethostbyname (pHostname ) ) )
		_sockAddress.sin_addr.s_addr = *( ( int*)hp->h_addr_list[0] ) ;
	else
		_sockAddress.sin_addr.s_addr = inet_addr ( pHostname ) ;
	_sockAddress.sin_port = htons ( port ) ;
	_addressLen = sizeof(_sockAddress) ;
}

int _ossSocket::bind_listen () {
	int rc = GHDB_OK ;
	int temp = 1 ;
	rc = setsockopt ( _fd, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(int) ) ;
	if ( rc ) { 
		printf("Failed to setsockopt SO_REUSERADDR, rc = %d", SOCKET_GETLASTERROR ) ;
	}
	rc = setSocketLi ( 1, 30 ) ;
	if ( rc ) {
		printf("Failed to setsocketopt SO_LINGER, rc = %d", SOCKET_GETLASTERROR ) ;
	}
	rc = ::bind ( _fd, (struct sockaddr*)&_sockAddress, _addressLen ) ;
	if ( rc ) {
		printf("Failed to bind socket, rc = %d", SOCKET_GETLASTERROR ) ;
		printf("_fd = %d", _fd) ;
		printf("_addressLen = %d", _addressLen) ;
		rc = GHDB_NETWORK ;
		goto error ;
	}
	rc = listen ( _fd, SOMAXCONN ) ;
	if ( rc ) {
		printf("Failed to listen socket, rc = %d", SOCKET_GETLASTERROR ) ;
		rc= GHDB_NETWORK ;
		goto error ;
	}
	done :
		return rc ;
	error :
		close () ;
		goto done ;
}

int _ossSocket::send ( const char *pMsg, int len, int timeout, int flags) {
	int rc = GHDB_OK ;
	int maxFD = _fd ;
	struct timeval maxSelectTime ;
	fd_set fds;
	
	maxSelectTime.tv_sec = timeout / 1000000 ;
	maxSelectTime.tv_usec = timeout % 1000000 ;
	// if len == 0, then let's just return
	if ( 0== len) {
		return GHDB_OK ;
	}
	// wait loop untile socket is ready
	while ( true ) {
		FD_ZERO ( &fds ) ;
		FD_SET ( _fd, &fds ) ;
	rc = select ( maxFD + 1, NULL, &fds, NULL, timeout>=0?&maxSelectTime:NULL) ;
		if ( 0 == rc ) {
			// timeout
			rc = GHDB_TIMEOUT ;
			goto done;
		}
		// if rc < 0, something wrong
		if ( 0 > rc ) {
			rc = SOCKET_GETLASTERROR ; 
			// if we fauled due to interrupt, let's continue
			if ( EINTR == rc ) {
				continue ;
			}
			printf("Failed to select from socket, rc = %d", rc ) ;
			rc = GHDB_NETWORK ;
			goto error ;
		}
		if ( FD_ISSET ( _fd, &fds ) )
			break ;
	}
	while ( len > 0) {
		rc = ::send ( _fd, pMsg, len, MSG_NOSIGNAL | flags ) ;
		if ( -1 == rc ) {
			printf("Failed to send, rc = %d", SOCKET_GETLASTERROR ) ;
			rc = GHDB_NETWORK ;
			goto error ;
		}
		len -= rc ;
		pMsg += rc;
	}
	rc = GHDB_OK ;
	done :
		return rc ;
	error :
		goto done ;
}

bool _ossSocket::isConnected () {
	int rc = GHDB_OK ;
	rc = ::send ( _fd, "", 0, MSG_NOSIGNAL ) ;
	if ( 0 > rc)
		return false ;
	return true ;
}

#define MAX_RECV_RETRIES 5
int _ossSocket::recv ( char *pMsg, int len, int timeout, int flags ) {
	int rc = GHDB_OK ;
	int retries = 0;
	int maxFD = _fd ;
	struct timeval maxSelectTime ;
	fd_set fds ;
	
	if ( 0== len )
		return GHDB_OK ;
	maxSelectTime.tv_sec = timeout / 1000000 ;
	maxSelectTime.tv_usec = timeout % 1000000 ;
	while ( true ) {
		FD_ZERO ( &fds ) ;
		FD_SET ( _fd, &fds ) ;
		rc = select ( maxFD + 1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL) ;
		// 0 means timeout
		if ( 0 == rc ) {
			rc = GHDB_TIMEOUT ;
			goto done ;
		}
		// if < 0 something wrong
		if ( 0 > rc ) {
			rc = SOCKET_GETLASTERROR ;
			if ( EINTR == rc )
				continue ;
			printf("Failed to select from socket, rc = %d", rc ) ;
			rc = GHDB_NETWORK ;
			goto error ;
		}
		if ( FD_ISSET ( _fd, &fds ) )
			break ;
	}
	while ( len > 0) {
		rc = ::recv ( _fd, pMsg, len, MSG_NOSIGNAL | flags ) ;
		if ( rc > 0 ) {
			if ( flags & MSG_PEEK )
				goto done ;
			len -= rc ;
			pMsg += rc ;
		}
		else if ( rc == 0 ) {
			printf("Peer unexpected shutdown") ;
			rc = GHDB_NETWORK_CLOSE ;
			goto error ;
		}
		else {
			rc = SOCKET_GETLASTERROR ;
			if ( ( EAGAIN == rc || EWOULDBLOCK == rc ) && _timeout > 0 ) {
				printf("Recv() timeout: rc = %d", rc ) ;
				rc = GHDB_NETWORK ;
				goto error ;
			}
			if ( (EINTR == rc ) && (retries < MAX_RECV_RETRIES ) ) {
				retries ++ ;
				continue ;
			}
			printf("Recv() Failed: rc = %d", rc ) ;
			rc = GHDB_NETWORK ;
			goto error ;
		}
	}
	rc = GHDB_OK ;
	done :
		return rc ;
	error :
		goto done ;
}	

int _ossSocket::recvNF ( char *pMsg, int len, int timeout ) {
	int rc = GHDB_OK ;
	int retries = 0;
	int maxFD = _fd ;
	struct timeval maxSelectTime ;
	fd_set fds ;
	
	if ( 0== len )
		return GHDB_OK ;
	maxSelectTime.tv_sec = timeout / 1000000 ;
	maxSelectTime.tv_usec = timeout % 1000000 ;
	while ( true ) {
		FD_ZERO ( &fds ) ;
		FD_SET ( _fd, &fds ) ;
		rc = select ( maxFD + 1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL) ;
		// 0 means timeout
		if ( 0 == rc ) {
			rc = GHDB_TIMEOUT ;
			goto done ;
		}
		// if < 0 something wrong
		if ( 0 > rc ) {
			rc = SOCKET_GETLASTERROR ;
			if ( EINTR == rc )
				continue ;
			printf("Failed to select from socket, rc = %d", rc ) ;
			rc = GHDB_NETWORK ;
			goto error ;
		}
		if ( FD_ISSET ( _fd, &fds ) )
			break ;
	}
	rc = ::recv ( _fd, pMsg, len, MSG_NOSIGNAL ) ;
	if ( rc > 0 ) {
		len -= rc ;
	}
	else if ( rc == 0 ) {
		printf("Peer unexpected shutdown") ;
		rc = GHDB_NETWORK_CLOSE ;
		goto error ;
	}
	else {
		rc = SOCKET_GETLASTERROR ;
		if ( ( EAGAIN == rc || EWOULDBLOCK == rc ) && _timeout > 0 ) {
			printf("Recv() timeout: rc = %d", rc ) ;
			rc = GHDB_NETWORK ;
			goto error ;
		}
		if ( (EINTR == rc ) && (retries < MAX_RECV_RETRIES ) ) {
			retries ++ ;
		}
		printf("Recv() Failed: rc = %d", rc ) ;
		rc = GHDB_NETWORK ;
		goto error ;
	}
	rc = GHDB_OK ;
	done :
		return rc ;
	error :
		goto done ;
}

int _ossSocket::connect () {
	int rc = GHDB_OK ;
	rc = ::connect ( _fd, (struct sockaddr *)&_sockAddress, _addressLen ) ;
	if ( rc ) {
		printf("Failed to connect, rc = %d", SOCKET_GETLASTERROR) ;
		rc = GHDB_NETWORK ;
		goto error ;
	}
	rc = getsockname ( _fd, (struct sockaddr *)&_sockAddress, &_addressLen ) ;
	if ( rc ) {
		printf("Failed to get local address, rc = %d", rc ) ;
		rc = GHDB_NETWORK ;
		goto error ;
	}
	rc = getpeername ( _fd, (struct sockaddr *)&_peerAddress, &_peerAddressLen ) ;
	if ( rc ) {
		printf("Failed to get peer address, rc = %d", rc ) ;
		rc = GHDB_NETWORK ;
		goto error ;
	}
	done :
		return rc ;
	error :
		goto done ;
}

void _ossSocket::close () {
	if ( _init ) {
		int i = 0 ;
		i = ::close ( _fd ) ;
		if ( i < 0 ) {
			i = -1 ;
		}
		_init = false ;
	}
}

int _ossSocket::accept ( int *sock, struct sockaddr *addr, socklen_t *addrLen, int timeout ) {
	int rc = GHDB_OK ;
	int maxFD = _fd ;
	struct timeval maxSelectTime ;
	
	fd_set fds ;
	maxSelectTime.tv_sec = timeout / 1000000 ;
	maxSelectTime.tv_usec = timeout / 1000000 ;
	while ( true ) {
		FD_ZERO ( &fds ) ;
		FD_SET ( _fd, &fds ) ;
		rc = select ( maxFD + 1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL ) ;
		// 0 means timeout
		if ( 0 == rc ) {
			*sock  = 0 ;
			rc = GHDB_TIMEOUT ;
			goto done ;
		}
		// if < 0, means something wrong
		if ( 0 > rc ) {
			rc = SOCKET_GETLASTERROR ;
			// if we failed due to interrupt, let's continue
			if ( EINTR == rc ) 
				continue ;
			printf("Failed to select from socket, rc = %d", SOCKET_GETLASTERROR ) ;
			rc = GHDB_NETWORK ;
			goto error ;
		}
		// if the socket we interrupted is noe receiving anything, let's continue
		if ( FD_ISSET ( _fd, &fds ) ) 
			break ;

	}
	rc = GHDB_OK ;
	*sock = ::accept ( _fd, addr, addrLen ) ;
	if ( -1 == *sock ) {
		printf("Failed to accept socket, rc = %d", SOCKET_GETLASTERROR ) ;
		rc = GHDB_NETWORK ;
		goto error ;
	}
	done :
		return rc ;
	error : 
		goto done ;
}

int _ossSocket::disableNagle () {
	int rc = GHDB_OK ;
	int temp = 1 ;
	rc = setsockopt ( _fd, IPPROTO_TCP, TCP_NODELAY, (char *) &temp, sizeof ( int ) ) ;
	if ( rc ) {
		printf("Failed to setsockopt, rc = %d", SOCKET_GETLASTERROR ) ;
	}
	rc = setsockopt ( _fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &temp, sizeof ( int ) ) ;
	if ( rc )
		printf("Failed to setsockopt, rc = %d", SOCKET_GETLASTERROR ) ;
	return rc ;
}

unsigned int _ossSocket::_getPort ( sockaddr_in *addr ) {
	return ntohs ( addr-> sin_port) ;
}

int _ossSocket::_getAddress ( sockaddr_in *addr, char *pAddress, unsigned int length ) {
	int rc = GHDB_OK ;
	length = length < NI_MAXHOST ? length : NI_MAXHOST ;
	rc = getnameinfo ( ( struct sockaddr * )addr, sizeof ( sockaddr ) , pAddress, length ,
			     NULL, 0, NI_NUMERICHOST ) ;
	if ( rc ) {
		printf("Failed to getnameinfo, rc = %d", SOCKET_GETLASTERROR ) ;
		rc = GHDB_NETWORK ;
		goto error ;
	}
	done : 
		return rc ;
	error :
		goto done ;
}

unsigned int _ossSocket::getLocalPort () {
	return _getPort ( &_sockAddress ) ;
}

unsigned int _ossSocket::getPeerPort () {
	return _getPort ( &_peerAddress ) ;
}

int _ossSocket::getLocalAddress ( char * pAddress, unsigned int length ) {
	return _getAddress ( &_sockAddress, pAddress, length ) ;
}

int _ossSocket::getPeerAddress ( char * pAddress, unsigned int length ) {
	return _getAddress ( &_peerAddress, pAddress, length ) ;
}

int _ossSocket::setTimeout ( int seconds ) {
	int rc = GHDB_OK ;
	struct timeval tv ;
	tv.tv_sec = seconds ;
	tv.tv_usec = 0 ;
	//windows take milliseconds as parameter
	// but linux takes timeval sa input
	
	rc = setsockopt ( _fd, SOL_SOCKET, SO_RCVTIMEO, ( char * ) &tv, sizeof ( tv ) ) ;
	if ( rc ) 
		printf("Failed to setsockopt, rc = %d", SOCKET_GETLASTERROR ) ;
	return rc ;
}

int _ossSocket::getHostName ( char *pName, int nameLen ) {
	return gethostname ( pName, nameLen ) ;
}

int _ossSocket::getPort ( const char *pServiceName, unsigned short &port ) {
	int rc = GHDB_OK ;
	struct servent *servinfo ;
	servinfo = getservbyname ( pServiceName, "tcp" ) ;
	if ( !servinfo ) 
		port = atoi ( pServiceName ) ;
	else
		port = (unsigned short)ntohs(servinfo->s_port);
	return rc;
}
