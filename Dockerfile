FROM ubuntu:latest


RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    python3-pip \
    git

WORKDIR /plus

COPY . .

RUN mkdir build

# Install conan
RUN pip3 install conan --break-system-packages

RUN conan profile detect --force

RUN ./configure.docker.sh dbg

# Build
RUN cmake --build ./build

RUN ./build/plus --version