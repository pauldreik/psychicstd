ARG GCC_VERSION=16
FROM gcc:${GCC_VERSION}
RUN apt-get update -q \
    && apt-get install -y -q cmake git \
    && rm -rf /var/lib/apt/lists/*
