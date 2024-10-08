# FISim: Future Internet Architecture Simulator

## Table of Contents

1) [Overview](#overview)
2) [Build Network](#build-network)
3) [Run FISim](#running-fisim)
4) [Cybertwin Network](#cybertwin-network)
5) [Content Centric Network](#content-centric-network)

## Overview

FISim is an open-source network simulator dedicated to FIA (Future Internet Architecture), based on [NS-3](https://www.nsnam.org).

## Build Network

FISim utilizes YAML-based configuration files to construct network topologies, thereby greatly simplifying the work of researchers and practitioners.

## Running FISim

Running FISim is quite straightforward; once you have defined the network topology, system configuration, and applications, you can easily use the commands provided by FISim to compile and run your simulation.

## Cybertwin Network

In cloud computing, the Internet is shifting from end-to-end to cloud-to-end connections. Concurrently, the rapid advancement of the Internet of Everything (IoE) and its applications is increasing mobile Internet traffic and services, challenging the existing end-to-end network architecture in terms of mobility, security, and availability. To address these challenges, the Cybertwin network architecture has been proposed. The Cybertwin network introduces the Cybertwin

>Cybertwin is a digital representation of humans or things in virtual cyberspace, created for functions such as communication assistance, network data logging, and digital asset ownership. It aims to replace the traditional end-to-end communication model with a cybertwin-based approach.

concept into networking and presents a new communication model. Additionally, it introduces a novel cloud network operating system and cloud operator to manage network resources. Key concepts of the Cybertwin network include infrastructure, the Cybertwin based communication model, the cloud network operating system, and the cloud operator. For more details, see~\cite{Cybertwin,cybertwin2}

## Content Centric Network

Content-Centric Networking (CCN) is an innovative approach to network architecture that is designed to make content the primary focus of the network, rather than the traditional host-centric model where communication is based on the location of the host . In CCN, data is named and addressed, and the network is responsible for delivering that data to the requester, regardless of where the data is located. This is achieved through a publish/subscribe model where consumers request data by name, and the network delivers the data from the nearest node that has it available, which can improve efficiency, especially for popular content that can be cached close to the consumer.

CCN includes features such as in-network caching, which allows for the storage of content at various points throughout the network, reducing the need to retrieve content from its original source each time it is requested. This can lead to significant improvements in latency and bandwidth usage. Additionally, CCN is designed to be more resilient and secure, as content can be verified and authenticated through digital signatures before it is processed by routers or consumers .

The architecture of CCN is such that it can naturally support scalability, efficiency, and security. It does this by decoupling content from its location, allowing networks to handle increased traffic by caching popular content closer to users, reducing the distance data must travel and balancing loads more effectively across the network . Security in CCN is inherently content-centric, with data packets including cryptographic signatures that ensure content integrity and authenticity, regardless of where the content is stored or how it is transmitted.

CCN has the potential to greatly enhance the way content is delivered over the internet, making it more suitable for modern applications such as streaming media, content delivery networks, and mobile applications where users frequently change locations . It represents a significant shift from traditional host-centric network architectures towards a more flexible, efficient, and secure content-centric networking approach .

