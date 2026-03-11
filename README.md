# SAS Communication Library

A lightweight **C++14 library implementing the physical communication layer for the SAS (Slot Accounting System) protocol**.

This project focuses on **deterministic, low-latency UART communication** required by SAS hosts and gaming machines. The library is designed for embedded and industrial environments where predictable timing and minimal dependencies are essential.

At the current stage the project implements **only the physical communication layer**, which provides the foundation for the future SAS link and protocol layers.

---

# Features

### Direct UART Hardware Access

The library communicates **directly with UART hardware registers** rather than using operating system serial APIs.

Benefits include:

* deterministic communication timing
* minimal overhead
* full control of UART configuration
* reliable detection of SAS signaling

This approach is particularly useful in embedded Linux or industrial PC environments where low-level access is available.

---

### SAS Wakeup Bit Support (9-Bit Emulation)

The SAS protocol uses a **9-bit serial format**:

* The **first byte** of a message contains a **wakeup bit**
* Remaining bytes contain normal data

Most UARTs do not support native 9-bit frames. This library emulates the wakeup bit using **MARK/SPACE parity**, allowing standard UART hardware to transport the required signaling.

The wakeup flag is preserved internally as part of the byte pipeline.

---

### Lock-Free SPSC Ring Buffers

The library uses **Single Producer / Single Consumer (SPSC) lock-free ring buffers** for both receive and transmit paths.

Two buffers are used:

* **RX Buffer** – UART → protocol layer
* **TX Buffer** – protocol layer → UART

Each element is stored as a `uint16_t`:

```text
bit 0..7  : data byte
bit 8     : SAS wakeup flag
```

Advantages:

* lock-free design
* deterministic performance
* constant-time operations
* cache-friendly memory layout

---

### Polling-Based UART Driver

The UART driver uses **high-frequency polling** instead of interrupts.

This approach was chosen because it provides:

* predictable latency
* simplified driver logic
* minimal interrupt jitter
* easier portability across embedded platforms

The I/O loop uses a **drain-then-fill strategy** to prioritize receiving data before transmitting new bytes.

---

# Architecture

The current architecture isolates the physical communication layer from higher SAS protocol logic.

```
                +---------------------------+
                |      Future SAS Layers    |
                |   (link + protocol)       |
                +------------+--------------+
                             |
                       RX / TX queues
                             |
                +------------v--------------+
                |      Physical Layer       |
                |                           |
                |   UART driver             |
                |   Wakeup bit handling     |
                |   Lock-free ring buffers  |
                +------------+--------------+
                             |
                +------------v--------------+
                |        UART Hardware      |
                +---------------------------+
```

This design allows the physical layer to remain **independent, testable, and reusable**.

---

# Data Flow

### Receive Path

```
UART RX
   ↓
hardware polling
   ↓
wakeup bit detection
   ↓
RX ring buffer
   ↓
future SAS link layer
```

### Transmit Path

```
future SAS link layer
   ↓
TX ring buffer
   ↓
UART driver
   ↓
hardware transmission
```

---

# Example Usage (Conceptual)

Future SAS layers will interact with the physical layer through the ring buffers:

```cpp
uint16_t word;

/* Receive byte */
if (rxBuffer.pop(word))
{
    uint8_t data = word & 0xFF;
    bool wakeup  = word & 0x100;
}

/* Transmit byte */
uint16_t tx = data;
if (wakeup)
    tx |= 0x100;

txBuffer.push(tx);
```

---

# Building the Project

The project uses **CMake** and requires a **C++14 compatible compiler**.

```
git clone https://github.com/<your-username>/saslib.git
cd saslib

mkdir build
cd build

cmake ..
make
```

---

# Project Status

Current implementation:

```
✓ UART physical layer
✓ SAS wakeup bit detection
✓ lock-free SPSC ring buffers
✓ polling communication loop
```

Planned future work:

```
• SAS link layer implementation
• message parser
• command dispatcher
• protocol state machine
```

---

# Contributing

Contributions are welcome.

Typical contributions include:

* improvements to UART driver robustness
* portability enhancements
* testing tools for SAS communication
* implementation of higher protocol layers

Please open an issue before submitting large changes so the design can be discussed.

---

# License

This project is licensed under the **Apache 2.0 License**.

---

# Author

Luiz Cressoni

