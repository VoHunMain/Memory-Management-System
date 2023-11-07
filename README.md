# Memory-Management-System
# MeMS (Memory Management System)

MeMS is a custom memory management system implemented in C. It provides dynamic memory allocation and deallocation for user programs. This README provides an overview of the MeMS system, its functions, and usage instructions.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Getting Started](#getting-started)
- [Initialization](#initialization)
## Introduction

MeMS is a custom memory management system designed to provide memory allocation and deallocation capabilities to user programs. It manages memory in a hierarchical structure, consisting of main nodes and sub-nodes. The system is designed to reuse memory segments efficiently and minimize the use of `mmap` system calls, which can be costly.

## Features

- Dynamic memory allocation and deallocation.
- Efficient reuse of memory segments.
- Detailed statistics about memory usage.
- Mapping of virtual addresses to physical addresses.
- Custom memory management structure.
  I have added a function which handles the edge cases of combininig consecutive holes into a single hole node.

## Getting Started

### Initialization

To use MeMS, you need to initialize the system. Call the `mems_init` function to set up the MeMS system. This function initializes the starting virtual address and prepares the main data structures.

```c
mems_init();
