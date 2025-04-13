FROM ubuntu:22.04

# Avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install required packages for OMNeT++
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    lld \
    gdb \
    bison \
    flex \
    perl \
    python3 \
    python3-pip \
    libpython3-dev \
    qtbase5-dev \
    qtchooser \
    qt5-qmake \
    qtbase5-dev-tools \
    libqt5opengl5-dev \
    libxml2-dev \
    zlib1g-dev \
    doxygen \
    graphviz \
    libwebkit2gtk-4.0-dev \
    xdg-utils \
    wget \
    xvfb \
    libxkbcommon-x11-0 \
    libgl1-mesa-glx \
    git \
    cmake \
    && apt-get clean

# Set up working directory
WORKDIR /project

# Get OMNeT++ and extract it
RUN wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-6.1.0/omnetpp-6.1.0-linux-x86_64.tgz \
    && tar xf omnetpp-6.1.0-linux-x86_64.tgz \
    && rm omnetpp-6.1.0-linux-x86_64.tgz

# Install Python dependencies
RUN cd /project/omnetpp-6.1 && \
    python3 -m pip install -r python/requirements.txt

# Configure and build OMNeT++ (using bash to source the environment)
RUN cd /project/omnetpp-6.1 && \
    echo "WITH_OSG=no" >> configure.user && \
    echo "WITH_OSGEARTH=no" >> configure.user && \
    echo "WITH_QTENV=no" >> configure.user && \
    bash -c "source setenv && ./configure && make -j$(nproc)"

# Set environment variables
ENV PATH="/project/omnetpp-6.1/bin:${PATH}"
ENV LD_LIBRARY_PATH="/project/omnetpp-6.1/lib:${LD_LIBRARY_PATH}"

# Set up script to source environment when container starts
RUN echo '#!/bin/bash\ncd /project/omnetpp-6.1 && source setenv && cd - > /dev/null && exec "$@"' > /entrypoint.sh \
    && chmod +x /entrypoint.sh

# Use entrypoint script to ensure environment is set up
ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]