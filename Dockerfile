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
    apt-transport-https \
    ca-certificates \
  && rm -rf /var/lib/apt/lists/* \
  && update-ca-certificates \

# NOTE: The following are disable for auto-builds of docker images for use
#       with the github CI workflow. Workflow containers must run as root.
  #&& groupadd -r netmeld && useradd -r -s /bin/false -g netmeld netmeld
#USER netmeld

