FROM ubuntu:22.04

# Avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install all dependencies for NS-3
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    python3 \
    python3-dev \
    pkg-config \
    sqlite3 \
    libsqlite3-dev \
    cmake \
    libboost-all-dev \
    libgsl-dev \
    git \
    gdb \
    valgrind \
    libgtk-3-dev \
    # Replace qt5-default with these packages
    qtbase5-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    # Rest of packages
    libxml2-dev \
    libpcap-dev \
    tcpdump \
    wireshark \
    doxygen \
    graphviz \
    imagemagick \
    vim \
    && apt-get clean

# Clone NS-3
RUN git clone https://gitlab.com/nsnam/ns-3-dev.git /ns-3

# Build NS-3 with only the modules needed
WORKDIR /ns-3
RUN ./ns3 configure --enable-modules=core,network,internet,applications,wifi,propagation,mobility,dsdv,dsr,aodv,olsr,flow-monitor,stats,netanim
RUN ./ns3 build

# Set up the project directory
WORKDIR /project
CMD ["/bin/bash"]