# CTSim: A Scalable and Flexible Network Simulator for Cybertwin Network

## Table of Contents

1) [Overview](#overview)
2) [Quick Start](#quick-start)
    - [Prerequisites](#prerequisites)
    - [Installation](#installation)
    - [Running CTSim](#running-ctsim)
3) [Build Network](#build-network)
4) [Cybertwin Network](#cybertwin-network)
5) [Content Centric Network](#content-centric-network)
6) [Reference](#reference)

## Overview

The **Cybertwin network** is a new network architecture that aims to replace the traditional end-to-end communication model with a cybertwin-based approach. The Cybertwin network introduces the Cybertwin concept into networking and presents a new communication model. Additionally, it introduces a novel cloud network operating system and cloud operator to manage network resources. Key concepts of the Cybertwin network include infrastructure, the Cybertwin based communication model, the cloud network operating system, and the cloud operator. For more details, see [[1](#ref1)].

To facilitate the research and development of the Cybertwin network, we have developed this network simulator called **CTSim**. CTSim is an open-source network simulator dedicated to the Cybertwin network, based on [NS-3](https://www.nsnam.org). CTSim utilizes YAML-based configuration files to construct network topologies, thereby greatly simplifying the work of researchers and practitioners. Running CTSim is quite straightforward; once you have defined the network topology, system configuration, and applications, you can easily use the commands provided by CTSim to compile and run your simulation.

## Quick Start

### Prerequisites

- [NS-3](https://www.nsnam.org) (version 3.33 or later)
- [Python](https://www.python.org) (version 3.6 or later)
- [CMake](https://cmake.org) (version 3.1 or later)

### Installation

1. Clone the repository:

```bash
git clone https://github.com/myk12/CTSim.git
```

2. Change to the CTSim directory:

```bash
cd CTSim
```

3. Install the required Python packages:

```bash
pip install -r requirements.txt
```

### Running CTSim

1. Define the network topology, system configuration, and applications in the YAML files.

2. Configure 

```bash
./ctsim configure
```

3. Compile the network:

```bash
./ctsim build
```

4. Run the simulation:

```bash
./ctsim run
```

5. Analyze the results:

```bash
./ctsim analyze
```

## Build Network

### Network Topology

Cybertwin network is a hierarchical network architecture that consists of three layers: **core**, **edge**, and **access**. 
- The core layer is the backbone of the network and connects the edge and access layers. 
- The edge layer connects the core and access layers and provides services to the access layer. 
- The access layer connects end devices to the network. 

The network topology is defined in a YAML file. The following is an example of a network topology file:

```yaml
cybertwin_network:
  # Core Layer
  core_layer:
    description: Core Layer of the Cybertwin Network
    nodes:
      - name: core_node1
        type: host_server
        connections:
          - target: core_node2
            network: 10.0.0.0/8
            data_rate: 10Gbps
            delay: 10ms

          - target: core_node3
            network: 11.0.0.0/8
            data_rate: 10Gbps
            delay: 10ms
    ...

  # Edge Layer
  edge_layer:
    description: Edge Layer of the Cybertwin Network
    nodes:
      - name: edge_node1
        type: host_server
        connections:
          - target: core_node1
            network: 100.10.0.0/16
            data_rate: 1Gbps
            delay: 20ms
    ...
      
  # Access Layer
  access_layer:
    description: Access Layer of the Cybertwin Network
    nodes:
      - name: access_net1
        type: end_cluster
        num_nodes: 4
        network_type: ethernet
        local_network: 192.168.1.0/24
        gateways:
          - target: edge_node1
            network: 200.1.0.0/16
            data_rate: 100Mbps
            delay: 20ms
    ...
```

In the network topology file, the `cybertwin_network` key is the root key that contains the core, edge, and access layers. Each layer has a `description` key that describes the layer. The `nodes` key contains a list of nodes in the layer. Each node has a `name` key that specifies the name of the node, a `type` key that specifies the type of the node, and a `connections` key that specifies the connections of the node. The `connections` key contains a list of connections of the node. Each connection has a `target` key that specifies the target node, a `network` key that specifies the network address of the connection, a `data_rate` key that specifies the data rate of the connection, and a `delay` key that specifies the delay of the connection.

For the `end_cluster` type, the `num_nodes` key specifies the number of nodes in the cluster, the `network_type` key specifies the network type of the cluster, the `local_network` key specifies the local network address of the cluster, and the `gateways` key specifies the gateways of the cluster. Each gateway has a `target` key that specifies the target node, a `network` key that specifies the network address of the gateway, a `data_rate` key that specifies the data rate of the gateway, and a `delay` key that specifies the delay of the gateway.

Users can define their own network topologies by modifying the network topology file without changing the source code.

### Applications

Applications are defined in a YAML file. The following is an example of an applications file:

```yaml

```

### System Configuration

The system configuration is defined in a YAML file. The following is an example of a system configuration file:

```yaml

```

## Reference

<a id="ref1"></a>[1] [Quan Yu, Jing Ren, et al . 2019. Cybertwin: An Origin of Next Generation Network Architecture. IEEE Wirel. Commun. 26, 6 (2019), 111–117.](https://ieeexplore.ieee.org/document/8910636)
