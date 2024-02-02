#ifdef _WIN32
	#define _WIN32_WINNT 0x501
#endif

#include "TCPStreamBuffer.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#include <WS2tcpip.h>
#endif
#include <cstring>

TCPStreamBuffer::TCPStreamBuffer(std::size_t bufferLength)
	: outLength_(bufferLength), inLength_(bufferLength)
{
	if (outLength_ == 0) outLength_ = 1;
	if (inLength_ == 0) inLength_ = 1;

	outBuffer_ = std::make_unique<char[]>(outLength_);
	inBuffer_ = std::make_unique<char[]>(inLength_);

	setg(inBuffer_.get(), inBuffer_.get(), inBuffer_.get());
	setp(outBuffer_.get(), outBuffer_.get() + outLength_);
#ifdef _WIN32	
	WinSockInitializer::Instance().Initialize();
#endif	
}

TCPStreamBuffer::TCPStreamBuffer(TCPStreamBuffer&& other)
	: outBuffer_(std::move(other.outBuffer_)),
	inBuffer_(std::move(other.inBuffer_)),
	outLength_(other.outLength_),
	inLength_(other.inLength_),
	socket_(other.socket_)
{
#ifdef _WIN32	
	other.socket_ = INVALID_SOCKET;
#else
	other.socket_ = -1;
#endif	
	other.outLength_ = 0;
	other.inLength_ = 0;
}

TCPStreamBuffer& TCPStreamBuffer::operator=(TCPStreamBuffer&& other) {
	if (this != &other) {
		outBuffer_ = std::move(other.outBuffer_);
		outLength_ = other.outLength_;
		other.outLength_ = 0;
		
		inBuffer_ = std::move(other.inBuffer_);
		inLength_ = other.inLength_;
		other.inLength_ = 0;
		
		socket_ = other.socket_;
#ifdef _WIN32		
		other.socket_ = INVALID_SOCKET;
#else
		other.socket_ = -1;
#endif	
	}
	return *this;
}

TCPStreamBuffer::~TCPStreamBuffer() {
	Disconnect();

	// Shut down WinSock2 if this is the last instance of the buffer.
	// If someone else has initialized WinSock2, that's fine; WSACleanup()
	// only decrements an internal counter if WSAStartup() has been called
	// more than once.
#ifdef _WIN32	
	WinSockInitializer::Instance().Shutdown();
#endif
}

bool TCPStreamBuffer::Connect(const std::string& hostname, const std::string& port) {
	// Disconnect from any existing connection.
	Disconnect();

	// Reset buffer pointers.
	setg(inBuffer_.get(), inBuffer_.get(), inBuffer_.get());
	setp(outBuffer_.get(), outBuffer_.get() + outLength_);

#ifdef _WIN32
	// Resolve hostname.
	addrinfo hints;
	addrinfo* result = nullptr;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result) != 0) {
		return false;
	}

	// Create the socket used for transmission.
	socket_ = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (socket_ == INVALID_SOCKET) {
		freeaddrinfo(result);
		return false;
	}

	// Attempt to connect to the destination.
	if (connect(socket_, result->ai_addr, result->ai_addrlen) != 0) {
		freeaddrinfo(result);
		closesocket(socket_);
		socket_ = INVALID_SOCKET;
		return false;
	}
#else
	// Resolve hostname.
    struct hostent* hp;
    struct sockaddr_in addr;
    int result;
    int flags;
    fd_set fds;
    struct timeval tv;
    int timeout_seconds = 5;

    hp = gethostbyname(hostname.c_str());
    if (hp == nullptr) {
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(stoi(port));
    addr.sin_addr.s_addr = *(unsigned long*)hp->h_addr;

	// Create the socket used for transmission.
	socket_ = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_ < 0) {
		//freeaddrinfo(result);
		return false;
	}

    flags = fcntl(socket_, F_GETFL, 0);
    fcntl(socket_, F_SETFL, flags | O_NONBLOCK);

	// Attempt to connect to the destination.
    result = connect(socket_, (struct sockaddr*)&addr, sizeof(addr));
    if(result < 0) {
        if (errno != EINPROGRESS) {
            goto fail; //Error connecting to socket
        } else {
            FD_ZERO(&fds);
            FD_SET(socket_, &fds);
            tv.tv_sec = timeout_seconds;
            tv.tv_usec = 0;
            result = select(socket_ + 1, NULL, &fds, NULL, &tv);
            if (result < 0) {
                goto fail; //Error connecting (select)
            }
            else if (result == 0) {
                goto fail; //Connection timed out
            }
            else {
                int err;
                socklen_t len = sizeof(err);
                if (getsockopt(socket_, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
                    goto fail; //Error (getsockopt)
                }
                if (err != 0) {
                    goto fail; //Delayed connection error
                }
            }
        }
    }

    fcntl(socket_, F_SETFL, flags);
    return true;
#endif

#ifdef _WIN32
	freeaddrinfo(result);
	return true;
#else
fail:
    close(socket_);
    socket_ = -1;
    return false;
#endif
}

bool TCPStreamBuffer::Connect(SocketType s) {
	// Disconnect from any existing connection.
	Disconnect();

	// Reset buffer pointers.
	setg(inBuffer_.get(), inBuffer_.get(), inBuffer_.get());
	setp(outBuffer_.get(), outBuffer_.get() + outLength_);
	
	// Set the new socket.
	socket_ = s;
	
	// Return true if a valid socket was passed.
	return socket_ != SocketInvalid;
}

void TCPStreamBuffer::Disconnect() {
	if (socket_ != SocketInvalid) {
		// 1. Flush the out buffer to send any remaining data.
		// 2. Call shutdown() to initiate a graceful disconnection request.
		// 3. Close the socket.
		sync();
		shutdown(socket_, SHUT_RDWR);
		close(socket_);
		socket_ = SocketInvalid;
	}
}

int TCPStreamBuffer::underflow() {
	// Underflow is called when there is nothing left in the input buffer.
	// Underflow() will then receive some data from the connection
	// and put it in the buffer, where a stream can access it.

	// We're not connected or we have no buffer, so return EOF.
	if (socket_ == SocketInvalid || !inBuffer_.get()) {
		return std::char_traits<char>::eof();
	}

	// Read as much as we can into the buffer.
	int bytesRead = recv(socket_, inBuffer_.get(), inLength_, 0);

	if (bytesRead <= 0) {
		Disconnect();
		return std::char_traits<char>::eof();
	}

	// Set the input buffer's pointers to reflect the change and return
	// the first character in the buffer.
	setg(inBuffer_.get(), inBuffer_.get(), inBuffer_.get() + bytesRead);
	return std::char_traits<char>::to_int_type(inBuffer_.get()[0]);
}

int TCPStreamBuffer::overflow(int c) {
	if (socket_ == SocketInvalid || !outBuffer_.get())
		return std::char_traits<char>::eof();

	// Flush the buffer and reset the put pointers.
	//

	if (sync() != 0) {
		Disconnect();
		return std::char_traits<char>::eof();
	}

	setp(outBuffer_.get(), outBuffer_.get() + outLength_);

	// Insert the new character.
	if (c != std::char_traits<char>::eof())
		sputc(c);

	return c;
}


int TCPStreamBuffer::sync() {
	// Flush the buffer and reset the put pointers.
	if (!Send_(outBuffer_.get(), pptr() - pbase()))
		return -1;

	setp(outBuffer_.get(), outBuffer_.get() + outLength_);
	return 0;
}

bool TCPStreamBuffer::Send_(char* buffer, std::size_t len) {
	if (socket_ == SocketInvalid) return false;

	// Send the contents of the buffer, repeating calls to
	// send() in case not everything gets sent at once.
	std::size_t bytesSent = 0;
	while (bytesSent < len) {
		int res = send(socket_, buffer + bytesSent, len - bytesSent, 0);
#ifdef _WIN32
		if (res != SOCKET_ERROR) {
#else		
		if (res > 0) {
#endif
			bytesSent += res;
		}
		else {
			return false;
		}
	}
	sentSuccess_ = true;
	return true;
}

#ifdef _WIN32
void WinSockInitializer::Initialize() {
	if (refCount_ == 0) {
		WSAData data;
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
			throw std::runtime_error("WinSockInitializer::Initialize");
		}
	}

	++refCount_;
}

void WinSockInitializer::Shutdown() {
	--refCount_;
	
	if (refCount_ == 0) {
		WSACleanup();
	}
}
#endif

bool TCPStreamBuffer::IsSent() { 
	return sentSuccess_; 
}

void TCPStreamBuffer::resetSend() {
	sentSuccess_ = false; 
}

void TCPStreamBuffer::SetTimeout(int timeout) {
	// Set the socket as non-blocking
	int flags = fcntl(socket_, F_GETFL, 0);
	fcntl(socket_, F_SETFL, flags | O_NONBLOCK);

	// Set the timeout using SO_RCVTIMEO and SO_SNDTIMEO socket options
	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	//setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

	// Reset the socket to blocking mode
	fcntl(socket_, F_SETFL, flags);
}

// Resize the input buffer. If the buffer has been deleted,
// such as after a move operation, a new one is created.
void TCPStreamBuffer::ReallocateInBuffer(std::size_t newSize) {
	if (newSize == 0) newSize = 1;
	
	if (inBuffer_.get()) {
		std::unique_ptr<char[]> buf = std::make_unique<char[]>(newSize);
		
		// get relative position of the current buffer position
		std::size_t cpos = gptr() - eback();
		if (cpos > newSize) cpos = newSize;
		
		std::size_t len = std::min(newSize, inLength_);
		std::memcpy(buf.get(), inBuffer_.get(), len);
		inBuffer_ = std::move(buf);
		
		// reset get pointers
		setg(inBuffer_.get(), inBuffer_.get() + cpos, inBuffer_.get() + newSize);
	}
	else {
		inBuffer_ = std::make_unique<char[]>(newSize);
		setg(inBuffer_.get(), inBuffer_.get(), inBuffer_.get() + newSize);
	}
	
	inLength_ = newSize;
}

// Resize the out buffer. If the buffer has been deleted,
// such as after a move operation, a new one is created.
void TCPStreamBuffer::ReallocateOutBuffer(std::size_t newSize) {
	if (newSize == 0) newSize = 1;
	
	if (outBuffer_.get()) {
		std::unique_ptr<char[]> buf = std::make_unique<char[]>(newSize);
		std::size_t len = std::min(newSize, outLength_);
		std::memcpy(buf.get(), outBuffer_.get(), len);
		
		outBuffer_ = std::move(buf);
		
	}
	else {
		outBuffer_ = std::make_unique<char[]>(newSize);
	}
	
	outLength_ = newSize;
	setp(outBuffer_.get(), outBuffer_.get() + newSize); // reset put pointers
}