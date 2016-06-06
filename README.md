# trdp_proxy
ThinkOS Remote Debugger Protocol Proxy

This repository contains the source code and test utilities for a TRDP proxy.

The proxy runs in a local PC, connects to an embedded system running ThinkOS (target) using a local serial or USB port and performs these tasks:
 - autheticates a debug session by requesting a session initiation and forwarding a challenge to a remote authentication server for secure debugging.
 - allow local or remote debug sessions using GDB by implementing a GDB RSP stack who listens on a TCP port. The proxy translates GDB requests from and to TRDP RPC calls.
 - provide a terminal connection to access the ThinkOS console in the target.
 - provide ThinkOS kernel information such as CPU utilization, mutex locks, threads status.
 - 
