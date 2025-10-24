# ⚡ GPU Trigger Character Device Driver

[![Linux](https://img.shields.io/badge/OS-Linux-blue?logo=linux)](https://www.kernel.org/)
[![CUDA](https://img.shields.io/badge/CUDA-Enabled-green?logo=nvidia)](https://developer.nvidia.com/cuda-zone)
[![Kernel](https://img.shields.io/badge/Driver-Kernel%20Module-orange)](https://www.kernel.org/doc/html/latest/driver-api/index.html)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build](https://img.shields.io/badge/Build-Make-success)](https://www.gnu.org/software/make/)

## 📖 Overview

A high-performance **Linux character device driver** that enables direct **GPU computation triggering** from kernel space using NVIDIA CUDA. The driver supports three distinct trigger modes for flexible kernel-user space interaction and heterogeneous computing.

### 🎯 Key Features
- **Three Trigger Modes**: IRQ-driven, IOCTL-based, and sysfs-configurable
- **Kernel-User Space Bridge**: Seamless communication between kernel modules and CUDA applications
- **Real-time GPU Computation**: Direct GPU workload triggering from kernel context
- **Dynamic Mode Switching**: Runtime configuration via sysfs interface

## 🏗️ Architecture

### Trigger Modes
1. **Mode 0 (IRQ-driven)**: Kernel produces results after interrupt handling
2. **Mode 1 (Sysfs + IOCTL)**: User writes to sysfs and triggers GPU work via ioctl
3. **Mode 2 (Kernel-driven)**: User writes to sysfs, kernel executes GPU work directly

### System Integration
```
User Space          │    Kernel Space          │    GPU
────────────────────┼──────────────────────────┼─────────────────
CUDA Application ←→ │ Character Device Driver ←→ │ NVIDIA GPU
                    │ Sysfs Interface          │ CUDA Runtime
                    │ IRQ Handling             │
```

## 🛠️ Technical Stack

| Component | Technology |
|-----------|------------|
| **Kernel Driver** | Linux Kernel Module (C) |
| **GPU Computing** | CUDA C++ with NVCC |
| **Communication** | Sysfs, IOCTL, Character Devices |
| **Hardware** | NVIDIA RTX 3050 (Ampere SM 86) |
| **Build System** | GNU Make |

## 🚀 Quick Start

### Prerequisites
- Linux kernel 4.15+
- NVIDIA GPU with CUDA support
- NVIDIA drivers and CUDA toolkit
- Kernel headers for module compilation

### Installation & Usage

```bash
# Build and load kernel module
cd driver
make
sudo insmod gpu_trigger_driver.ko

# Build CUDA user application
cd user
nvcc -o user_app user_app.cu -arch=sm_86

# Run in different modes
sudo ./user_app 0    # Mode 0: IRQ-based
sudo ./user_app 1    # Mode 1: Sysfs + IOCTL
sudo ./user_app 2    # Mode 2: Sysfs-triggered
```

### Example Output
```bash
[CUDA] Device 0: NVIDIA GeForce RTX 3050 6GB Laptop GPU
gpu_trigger_user: running for mode 1
[Driver result] GPU computation done! jiffies=4299973232
[CUDA] N=1048576 sample: C[0]=0.000000, C[1048575]=0.000000
```

## 📁 Project Structure

```
gpu_trigger_project/
├── driver/                          # Kernel driver implementation
│   ├── gpu_trigger_driver.c         # Main driver source
│   ├── Makefile                     # Kernel module build configuration
│   └── Kbuild                       # Kernel build system config
├── user/                            # CUDA user-space application
│   ├── user_app.cu                  # CUDA application source
│   └── c_cpp_properties.json        # IDE configuration
├── docs/                            # Documentation
│   └── architecture.md              # Detailed architecture docs
├── tests/                           # Test suites
│   ├── integration/                 # Integration tests
│   └── performance/                 # Performance benchmarks
├── LICENSE                          # MIT License
└── README.md                        # This file
```

## 🔧 Development

### Building from Source
```bash
# Clone repository
git clone https://github.com/Abhinavcodez/gpu-trigger-driver.git
cd gpu-trigger-driver

# Build kernel module
cd driver && make

# Build user application  
cd ../user && make
```

### Debugging
```bash
# Check kernel messages
dmesg | tail -20

# Monitor sysfs entries
cat /sys/kernel/gpu_trigger/mode

# Debug with dynamic printk
echo 8 > /proc/sys/kernel/printk
```

## 📊 Performance

The driver achieves:
- **Sub-millisecond** kernel-to-GPU trigger latency
- **Zero-copy** data transfer optimization
- **Configurable** buffer sizes for different workload requirements

## 🔮 Future Enhancements

- [ ] **Multi-GPU Support**: Load balancing across multiple GPUs
- [ ] **Performance Profiling**: CUDA events integration for detailed timing
- [ ] **Hardware Integration**: Real GPIO interrupt support
- [ ] **Security Enhancements**: IOMMU and memory protection
- [ ] **Container Support**: Docker and Kubernetes deployment

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Abhinav Kumar Maurya**
- GitHub: [@Abhinavcodez](https://github.com/Abhinavcodez)
- Email: abhinavkm.it.22@nitj.ac.in

## 🙏 Acknowledgments

- Linux Kernel Documentation
- NVIDIA CUDA Toolkit Team
- Linux Driver Development Community
