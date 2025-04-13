## Docker Development Environment

### First Time Setup
1. Install Docker on your system:
   - [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop)
   - [Docker Desktop for Mac](https://www.docker.com/products/docker-desktop)
   - [Docker Engine for Linux](https://docs.docker.com/engine/install/)
2. Clone the repository and navigate to the project directory
3. Run `docker login` 
4.Run `docker-compose build` to build the OMNeT++ 6.1.0 container

### Daily Development
1. Start the container: `docker-compose up -d`
2. Connect to the container: `docker-compose exec omnetpp bash`
3. Inside the container, work on the project:
   - Run OMNeT++ commands like normal
   - Build with `make`
   - Run simulations
4. Exit the container with `exit` when done
5. Stop the container: `docker-compose down`

### For GUI Applications

#### Linux
1. Before starting Docker, run: `xhost +local:docker`
2. Start container normally
3. Launch OMNeT++ with `omnetpp` command inside the container

#### Windows
1. Install [VcXsrv Windows X Server](https://sourceforge.net/projects/vcxsrv/)
2. Run XLaunch from the Start menu
3. Select "Multiple windows" with Display number 0
4. Select "Start no client"
5. Check "Disable access control" in Extra settings
6. Click Next and Finish
7. Find your IP address using `ipconfig` in Command Prompt
8. Create a file named docker-compose.override.yml with:
    version: '3'
    services:
  omnetpp:
    environment:
      - DISPLAY=your-ip-address:0.0

#### macOS
1. Install [XQuartz](https://www.xquartz.org/)
2. Launch XQuartz and go to Preferences → Security
3. Check "Allow connections from network clients" and restart XQuartz
4. Run `xhost +localhost` in Terminal
5. Get your IP with `ifconfig en0 | grep inet | awk '$1=="inet" {print $2}'`
6. Add to docker-compose.override.yml:
version: '3'
services:
  omnetpp:
    environment:
      - DISPLAY=your-ip-address:0
7. Start Docker and run OMNeT++