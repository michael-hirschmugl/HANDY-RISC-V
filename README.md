# HANDY-RISC-V

**Hands-on Architecture, Notably DIY RISC-V**

A minimalist RV32I simulator written in C++ for people who want to feel
the inner workings of a CPU.

------------------------------------------------------------------------

## About

HANDY-RISC-V is a from-scratch implementation of a RISC-V RV32I core and
memory model in modern C++.\
The goal is not performance. The goal is understanding.

This project exists to make CPU architecture tangible: instruction
decoding, register files, control flow, memory access, and the structure
of a load/store ISA --- all implemented explicitly and transparently.

No frameworks. No hidden abstractions. Just logic.

------------------------------------------------------------------------

## Learning Philosophy

This simulator is built using only one authoritative resource:

RISC-V Unprivileged ISA Specification\
https://docs.riscv.org/reference/isa/\_attachments/riscv-unprivileged.pdf

The intent is to follow the specification directly and treat it as the
single source of truth.\
No secondary tutorials, no prebuilt emulators, no external decoding
libraries.

If it runs, it runs because the spec says so.

------------------------------------------------------------------------

## Current State

-   RV32I base instruction set (work in progress)
-   Simple memory model
-   Single-cycle execution model
-   Clean separation between core and memory components

------------------------------------------------------------------------

## Roadmap

This project is intentionally evolving toward deeper architectural
exploration.

### 1. Pipelined Processor

The long-term goal is to transform the current single-cycle model into a
pipelined processor implementation.\
This will include:

-   Stage separation (IF, ID, EX, MEM, WB)
-   Hazard detection
-   Forwarding
-   Control hazards and branch handling

The objective is to move from a functional simulator to an architectural
machine model.

------------------------------------------------------------------------

### 2. Cache Implementation

A major focus of this project is understanding caches deeply ---
mechanically and structurally.

Planned exploration:

-   Direct-mapped cache
-   Set-associative cache
-   Replacement strategies
-   Write policies (write-through vs write-back)
-   Performance visualization

Caches are where theoretical architecture meets real-world latency.\
Understanding them requires building them.

------------------------------------------------------------------------

### 3. Graphical Interface

Eventually, this simulator will gain a GUI to visualize:

-   Pipeline state
-   Register contents
-   Memory layout
-   Cache behavior
-   Instruction flow

There is consideration of building a polished visualization layer in
Unity, turning the simulator into an interactive architectural
exploration tool.

------------------------------------------------------------------------

## Why This Exists

Modern CPUs feel mystical because they are invisible.

But a CPU is just a deterministic state machine executing well-defined
rules.\
Once you implement one yourself, the mystery becomes structure --- and
structure becomes intuition.

HANDY-RISC-V is an attempt to bridge that gap.

------------------------------------------------------------------------

## Build

Currently built using a simple Makefile:

make\
./build/meinprogramm

------------------------------------------------------------------------

## Future Vision

What starts as a minimal simulator can evolve into:

-   A cycle-accurate teaching tool
-   A cache experimentation platform
-   A pipeline visualization engine
-   A playground for architectural ideas

The long-term goal is not just to simulate instructions --- but to
simulate understanding.
