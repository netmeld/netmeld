FROM debian:testing-slim

RUN apt update \
  && apt install --assume-yes --no-install-recommends \
    debconf \
    build-essential \
    cmake \
    make \
    gcc \
    g++ \
    git \
    help2man \
    pandoc \
    libboost-date-time-dev \
    libboost-iostreams-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-test-dev \
    libpqxx-dev \
    libpugixml-dev \
    libpcap0.8-dev \
    python \
  && rm -rf /var/lib/apt/lists/* \
  && groupadd -r netmeld && useradd -r -s /bin/false -g netmeld netmeld

USER netmeld
