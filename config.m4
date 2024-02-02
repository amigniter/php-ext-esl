PHP_ARG_ENABLE([esl],
  [whether to enable esl support],
  [AS_HELP_STRING([--enable-esl],
    [Enable esl support])],
  [no])

if test "$PHP_ESL" != "no"; then

  AC_DEFINE(HAVE_ESL, 1, [ Have esl support ])

  PHP_NEW_EXTENSION(esl, esl.cpp \
    src/php_ESLevent.cpp \
    src/ESLevent.cpp \
    src/ESLconnection.cpp \
    src/php_ESLconnection.cpp \
    src/ESLsocket.cpp \
    src/tcp-stream-buffer/TCPStreamBuffer.cpp \
    src/cJSON/cJSON.c \
    src/ESLserver.cpp \
    src/php_ESLserver.cpp, $ext_shared,,,cxx)

  PHP_REQUIRE_CXX()
fi
