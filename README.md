# Ad-Hoc Network Routing Simulation Project

This repository contains our implementation and comparison of DSDV, DSR, and GPSR routing protocols for ad-hoc networks using NS-3. This README explains how to set up the project and work with NS-3 on any operating system.

## 🚀 Quick Start

### Step 0: Install docker 

### Step 1: Clone the Repository

```bash
# Clone the main repo
git clone https://gitlab.liu.se/noalj314/tdde35-project-noalj314-oscca863-kevma271-malbe283.git
cd tdde35-project-noalj314-oscca863-kevma271-malbe283

# Initialize and download the NS-3 submodule
git submodule update --init --recursive
```

### Step 2: Build NS-3 with docker (Only Needed Once)
```bash
docker-compose build
```

### Step 3: Run NS-3 with docker
```bash
docker-compose up -d

docker exec -it ns3-dev bash # this will open a bash shell inside the docker container 

```
Now you should be inside the docker container and the ns3 files should be built. 

### Step 4: build your stuff 


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

## Running Simulations

```bash
 create a bash script to run the simulation see simpletest.sh in the project root 
```

## 📚 Documentation Resources

- [NS-3 Documentation](https://www.nsnam.org/documentation/)
- [NS-3 Tutorial](https://www.nsnam.org/docs/tutorial/html/)
- [Ad-Hoc Network Basics](https://www.researchgate.net/publication/221454408_Wireless_Ad-Hoc_Networks_An_Overview)

