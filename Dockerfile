FROM ubuntu:16.04

RUN dpkg --add-architecture i386 &&                                            \
    apt update -y && apt upgrade -y &&                                         \
    apt install -y vim gcc-multilib make libc6-dev:i386 libsqlite3-dev:i386 && \
    useradd -m abob_admin

USER abob_admin
WORKDIR /home/abob_admin
COPY bin/abob_cloud ./

ENTRYPOINT ["/bin/sleep", "infinity"]
