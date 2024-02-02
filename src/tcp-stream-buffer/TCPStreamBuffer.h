#pragma once

#include <streambuf>

#ifdef _WIN32
	#include <WinSock2.h>
	typedef SOCKET SocketType;
	const SocketType SocketInvalid = INVALID_SOCKET;
#else
//linux
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
//end linux
	typedef int SocketType;
	const SocketType SocketInvalid = -1;
#endif

#include <string>
#include <memory>

class TCPStreamBuffer : public std::streambuf
{
public:
	// Create a new, unconnected TCP stream buffer.
	TCPStreamBuffer(std::size_t bufferLength = bufferLength_);

	// Move constructor and move-assignment.
	// The moved-from stream buffer should not be used
	// again.
	
	TCPStreamBuffer(TCPStreamBuffer&& other);
	TCPStreamBuffer& operator=(TCPStreamBuffer&& other);

	// Copy constructor and copy-assignment are disabled.
	TCPStreamBuffer(const TCPStreamBuffer&) = delete;
	TCPStreamBuffer& operator=(const TCPStreamBuffer&) = delete;

	// If this is the last instance to die, WSACleanup() is called.
	~TCPStreamBuffer();

	// Connect to the given hostname and port. Returns true
	// on success, false otherwise.
	bool Connect(const std::string& hostname, const std::string& port);

	// Connect to an existing connection on the given socket.
	// This disconnects from the current connection (if there is any).
	// Returns true on success, false otherwise.
	bool Connect(SocketType s);

	// Disconnect the buffer. Does nothing if not already connected.
	void Disconnect();

	// Returns true if the buffer holds a valid connection, false otherwise.
	bool IsConnected() const {return socket_ != SocketInvalid;};

	// Return the internal socket handle (WinSock2).
	// If not connected, the value is INVALID_SOCKET.
	SocketType SocketHandle() const { return socket_; }

	// Get the underlying out buffer.
	const char* OutBuffer(std::size_t* len = nullptr) const { 
		if (len) *len = epptr() - pbase();
		return outBuffer_.get(); 
	}

	// Get the underlying in buffer.
	const char* InBuffer(std::size_t* len = nullptr) const { 
		if (len) *len = egptr() - eback();
		return inBuffer_.get();
	}

	bool IsSent();
	void resetSend();
	void SetTimeout(int timeout);

	// Resize the in buffer. If the buffer has been deleted,
	// such as after a move operation, a new one is created.
	void ReallocateInBuffer(std::size_t newSize);
	
	// Resize the out buffer. If the buffer has been deleted,
	// such as after a move operation, a new one is created.
	void ReallocateOutBuffer(std::size_t newSize);

protected:
	virtual int underflow() override;
	virtual int overflow(int c = EOF) override;
	virtual int sync() override;

private:
    bool Send_(char* buffer, std::size_t len);
	int socket_ = SocketInvalid;
	// Default buffer length
	static const std::size_t bufferLength_ = 4096;

	std::unique_ptr<char[]> outBuffer_;
	std::size_t outLength_;

	std::unique_ptr<char[]> inBuffer_;
	std::size_t inLength_;
	//to track data being sent
	bool sentSuccess_;
};

#ifdef _WIN32
class WinSockInitializer {
public:
	void Initialize();
	void Shutdown();
	static WinSockInitializer& Instance() {static WinSockInitializer instance; return instance;}
	
private:
	int refCount_ = 0;
	
	WinSockInitializer() {}
};
#endif