<?php

class ESLconnection {
    /**
     * ESLconnection constructor.
     *
     * INBOUND ("host","port","password") strings
     *
     * Initializes a new instance of ESLconnection, and connects to the host $host on the port $port, and supplies $password to FreeSWITCH.
     * Intended only for an event socket in "Inbound" mode. In other words, this is only intended for the purpose of creating a connection to FreeSWITCH that is not initially bound to any particular call or channel.
     *
     * OUTBOUND($sockfd) int
     *
     * Initializes a new instance of ESLconnection, using the existing file number contained in $sockfd.
     * Intended only for Event Socket Outbound connections.
     * It will fail on Inbound connections, even if passed a valid inbound socket.
     *
     * @param int|string ...$args Variable number of arguments
     */
    public function __construct(...$args) {}

    /**
     * Get the socket descriptor of the connection
     *
     * Returns the UNIX file descriptor for the connection object, if the connection object is connected.
     * This is the same file descriptor that was passed to new($sockfd) when used in outbound mode.
     *
     * @return int The socket descriptor
     */
    public function socketDescriptor(): int {}

    /**
     * Check if the connection is currently established
     *
     * @return bool True if connected, false otherwise
     */
    public function connected(): bool {}

    /**
     * Get information about the connection as an ESLevent object
     *
     * getInfo() returns NULL when used on an "Event Socket Inbound" connection.
     *
     * @return ESLevent An ESLevent object representing connection information
     */
    public function getInfo(): ESLevent {}

    /**
     * Send a command to FreeSWITCH.
     *
     * @param string $command The command to send.
     * @return bool Returns true if successful, or false on failure.
     */
    public function send(string $command): bool {}

    /**
     * Send a command and receive an ESL event.
     *
     * @param string $command The command to send.
     * @return ESLevent|null Returns an ESLevent object if successful, or null on failure.
     */
    public function sendRecv(string $command): ?ESLevent {}

    /**
     * Send an ESLevent and return the response as an ESLevent object
     *
     * @param ESLevent $event The ESLevent to send
     *
     * @return ESLevent|null The response ESLevent or null on failure
     */
    public function sendEvent(ESLevent $event): ?ESLevent {}

    /**
     * Retrieve events based on event type and value.
     *
     * don't use this call if you want only myevents; you must use sendRecv("myevents") instead
     *
     * if you want to use noevents; you must use sendRecv("noevents") instead
     * @param string $event_type can have the value "plain" or "xml". Any other value specified for $event_type gets replaced with "plain".
     * @param string $value      The value associated with the events. Should be a valid value string.
     * @return ESLevent|null Returns an ESLevent object if successful, or null on failure.
     */
    public function events(string $event_type, string $value): ?ESLevent {}

    /**
     * Filter events based on header and value.
     *
     * Specify event types to listen for. Note, this is not a filter out but rather a "filter in," that is, when a * filter is applied only the filtered values are received.
     * Multiple filters on a socket connection are allowed.
     * @param string $header The header to filter events. Should be a valid header string.
     * @param string $value  The value associated with the header. Should be a valid value string.
     *                      Additional details: Specify the value to filter events based on the provided header.
     * @return ESLevent|null Returns an ESLevent object if events match the specified criteria, or null otherwise.
     *                      Additional details: The returned ESLevent contains information about the filtered event.
     */
    public function filter(string $header, string $value): ?ESLevent {}

    /**
     * Receive the next ESL event.
     *
     * @return ESLevent|null Returns the next event from FreeSWITCH. If no events are waiting, this call will block until an event arrives.
     * If any events were queued during a call to sendRecv(), then the first one will be returned, and removed from the queue, then the next event will be read from the connection in sequence.
     */
    public function recvEvent(): ?ESLevent {}

    /**
     * Receive the next ESL event within a specified timeout.
     *
     * @param int $timeout Timeout in milliseconds. Defaults to 0 milliseconds if not provided.
     *                     Additional details: Specify the maximum time to wait for an event.
     * @return ESLevent|null Returns the next ESLevent object if available within the timeout, or null if no events are received.
     *                      Additional details: The returned ESLevent contains information about the received event.
     */
    public function recvEventTimed(int $timeout = 0): ?ESLevent {}

    /**
     * Send an API command to the FreeSWITCH server.
     * This method blocks further execution until the command has been executed.
     *
     * @param string $cmd  The API command to execute. Should be a valid command string.
     * @param string|null $arg  Additional argument for the API command. Defaults to null if not provided.
     *                          Additional details: Specify an argument if required by the API command.
     * @return ESLevent|null Returns an ESLevent object if the API command is successful, or null on failure.
     *                      Additional details: The returned ESLevent contains information about the API execution.
     */
    public function api(string $cmd, ?string $arg = null): ?ESLevent {}

    /**
     * Send a background API command to the FreeSWITCH server to be executed in it's own thread.
     * This will be executed in its own thread, and is non-blocking.
     *
     * @param string $cmd  The API command to execute. Should be a valid command string.
     * @param string|null $arg  Additional argument for the API command. Defaults to null if not provided.
     *                          Additional details: Specify an argument if required by the API command.
     * @return ESLevent|null Returns an ESLevent object if the API command is successful, or null on failure.
     *                      Additional details: The returned ESLevent contains information about the API execution.
     */
    public function bgapi(string $cmd, ?string $arg = null): ?ESLevent {}

    /**
     * Execute a dialplan application and wait for a response from the server.
     * On socket connections not anchored to a channel (most of the time inbound), all three arguments are required -- $uuid specifies the channel to execute the application on.
     *
     * @param string $app   The application to execute. Should be a valid application string.
     * @param string $arg   Arguments for the application. Should be a valid argument string.
     * @param string $uuid  The UUID to associate with the executed application. Should be a valid UUID string.
     * @return ESLevent|null Returns an ESLevent object if the application is successfully executed, or null on failure.
     *                      Additional details: The returned ESLevent contains information about the executed application.
     */
    public function execute(string $app, string $arg, string $uuid): ?ESLevent {}

    /**
     * Execute a dialplan application and do not wait for a response from the server.
     * This works by causing the underlying call to execute() to append "async: true" header in the message sent to the channel.
     *
     * @param string $app   The application to execute. Should be a valid application string.
     * @param string $arg   Arguments for the application. Should be a valid argument string.
     * @param string $uuid  The UUID to associate with the executed application. Should be a valid UUID string.
     * @return ESLevent|null Returns an ESLevent object if the application is successfully executed, or null on failure.
     *                      Additional details: The returned ESLevent contains information about the executed application.
     */
    public function executeAsync(string $app, string $arg, string $uuid): ?ESLevent {}

    /**
     * Force sync mode on for a socket connection.
     *
     * Specifically, calling setEventLock(1) operates by causing future calls to execute() to include the
     * "event-lock: true" header in the message sent to the channel. Other event socket library routines are
     * not affected by this call.
     *
     * @param bool $value The value should be 1 to force sync mode, and 0 not to force it.
     *                    This command has no effect on outbound socket connections that are not set to
     *                    "async" in the dialplan, since these connections are already set to sync mode.
     * @return bool Returns true if the event lock is successfully set, false otherwise.
     */
    public function setEventLock(bool $value): bool {}

    /**
     * Force async mode on for a socket connection.
     *
     * @param bool $value The value should be 1 to force async mode, and 0 not to force it.
     *                    This command has no effect on outbound socket connections that are set to "async"
     *                    in the dialplan and inbound socket connections, since these connections are already
     *                    set to async mode on.
     * @return bool Returns true if the event lock is successfully set, false otherwise.
     *              Additional details: Specifically, calling setAsyncExecute(1) operates by causing future calls to execute() to include the "async: true" header in the message sent to the channel. Other event socket library routines are not affected by this call.
     */
    public function setAsyncExecute(bool $value): bool {}


}

class ESLevent {
    /**
     * Constructor
     *
     * @param string $name      Event name (required)
     * @param string $subclass  Subclass name (optional)
     */
    public function __construct(string $name, ?string $subclass = null) {}

    /**
     * Serialize the event
     *
     * @param string $type Serialization type ('xml', 'json', plain')
     *
     * @return string The serialized event
     */
    public function serialize(?string $type = 'plain'): string {}

    /**
     * Set the priority of the event
     *
     * @param string $priority Priority value ("LOW", "HIGH", "NORMAL")
     *
     * @return bool true on success, false on failure
     */
    public function setPriority(string $priority): bool {}

    /**
     * Get the value of a header by name from an event object
     *
     * @param string $headerName Header name
     *
     * @return string|null The header value or null if not found
     */
    public function getHeader(string $headerName): ?string {}

    /**
     * Get the body of the event
     *
     * @return string The event body
     */
    public function getBody(): string {}

    /**
     * Get the type of the event
     *
     * @return string The event type
     */
    public function getType(): string {}

    /**
     * Add a body to the event
     *
     * @param string $value The body to add
     *
     * @return bool True on success, false on failure
     */
    public function addBody(string $value): bool {}

    /**
     * Add a header to the event
     *
     * This can be called multiple times for the same event object.
     *
     * @param string $headerName Header name
     * @param string|null $value Header value (optional)
     *
     * @return bool True on success, false on failure
     */
    public function addHeader(string $headerName, ?string $value = null): bool {}

    /**
     * Delete a header from the event
     *
     * @param string $headerName Header name
     *
     * @return bool True on success, false on failure
     */
    public function delHeader(string $headerName): bool {}

    /**
     * Get the value of the first header in the event
     *
     * Sets the pointer to the first header in an event object, and returns it's key name.
     * This must be called before nextHeader is called.
     *
     * @return string|null The header value or null if no headers
     */
    public function firstHeader(): ?string {}

    /**
     * Get the value of the next header in the event
     *
     * Moves the pointer to the next header in an event object, and returns it's key name.
     * firstHeader must be called before this method to set the pointer.
     * If you're already on the last header when this method is called, then it will return NULL.
     *
     * @return string|null The header value or null if no more headers
     */
    public function nextHeader(): ?string {}
}

class ESLserver
{
    /**
     * ESLserver constructor.
     *
     * @param string $host
     * @param string $port
     */
    public function __construct(string $host, string $port) {}

    /**
     * Accept a new child socket connection.
     *
     * @return int|null New child socket file descriptor or null on failure.
     */
    public function accept(): ?int {}

    /**
     * Close a child socket connection.
     *
     * @param int $clientSock The file descriptor of the child socket to close.
     *
     * @return void
     */
    public function close(int $clientSock): void {}

}
