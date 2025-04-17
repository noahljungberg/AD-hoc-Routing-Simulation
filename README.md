# Ad-Hoc Network Routing Simulation Project

This repository contains our implementation and comparison of DSDV, DSR, and GPSR routing protocols for ad-hoc networks using NS-3. This README explains how to set up the project and work with NS-3 on any operating system.

##  Quick Start

### Step 0: Install docker 

### Step 1: Clone the Repository

```bash
# Clone the main repo
git clone https://gitlab.liu.se/noalj314/tdde35-project-noalj314-oscca863-kevma271-malbe283.git
cd tdde35-project-noalj314-oscca863-kevma271-malbe283

# Initialize and download the NS-3 submodule
git submodule update --init --recursive
```

### Step 2: Build docker (Only Needed Once)
```bash
docker-compose build
```

### Step 3: Run NS-3 with docker
```bash
docker-compose up -d

docker exec -it ns3-dev bash # this will open a bash shell inside the docker container 

```

### Step 4: Build NS3 build files (Only Needed Once)
```bash
    mkdir /project/build
    cd /project/build
    cmake .. -DNS3_CXX_STANDARD=23  # maybe not needed not sure but do it anyways
    cmake --build .
```

Now the entire project including ns3 is built and ready to run. To run just do ./tdde35-runner in the build folder

## Running Simulations
To run the simulation, make necessary changes to cmakefiles.txt if needed, then;

### Step 1: Boot into docker 
```bash
docker-compose up -d
docker-compose  exec ns3-dev bash

```
### Step 2: Build
```bash
    cd build 
    cmake .. -DNS3_CXX_STANDARD=23  # maybe not needed not sure but do it anyways
    cmake --build .
```

### Step 3: Run your executable
```bash
    ./tdde35-runner  # example maybe different depending on cmakefiles.txt
```


## Updating NS-3

If we need to update the NS-3 submodule:

```bash
cd external/ns-3
git checkout master
git pull
cd ../..
git add external/ns-3
git commit -m "Update NS-3 submodule"
```

Then everyone else needs to run:

```bash
git pull
git submodule update --recursive
```

## 📚 Documentation Resources

- [NS-3 Documentation](https://www.nsnam.org/documentation/)
- [NS-3 Tutorial](https://www.nsnam.org/docs/tutorial/html/)
- [Ad-Hoc Network Basics](https://www.researchgate.net/publication/221454408_Wireless_Ad-Hoc_Networks_An_Overview)

