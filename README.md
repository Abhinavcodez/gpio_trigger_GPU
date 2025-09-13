# ⚡ GPU Trigger Project

[![Linux](https://img.shields.io/badge/OS-Linux-blue?logo=linux)](https://www.kernel.org/) 
[![CUDA](https://img.shields.io/badge/CUDA-Enabled-green?logo=nvidia)](https://developer.nvidia.com/cuda-zone) 
[![Kernel Module](https://img.shields.io/badge/Driver-Kernel%20Module-orange)](https://www.kernel.org/doc/html/latest/driver-api/index.html)
[![Build](https://img.shields.io/badge/Build-Make-success)](#)

---

## 📌 Overview
This project demonstrates **kernel ↔ user-space interaction** for triggering **GPU computation** via **NVIDIA CUDA** in three modes:

1. **Mode 0 (IRQ-driven):** Kernel produces the result after IRQ handling.  
2. **Mode 1 (Sysfs + IOCTL):** User writes to sysfs and triggers GPU work with ioctl.  
3. **Mode 2 (Kernel-driven via sysfs):** User writes sysfs, kernel executes GPU work directly.  

It integrates:
- A **character device driver** for GPIO-like triggers.  
- A **CUDA-based user application**.  
- **Sysfs entries** to dynamically switch trigger modes.  

---

## 💡 From Idea → Final Product
- **Initial idea:** GPIO-triggered kernel driver.  
- **Next:** Add multiple trigger modes (sysfs, ioctl, IRQ).  
- **Extension:** Integrate CUDA to offload computation to GPU.  
- **Final product:** Fully working **Linux kernel module + CUDA user-space app** with dynamic mode switching.

---

## 🛠️ Tech Stack
- **Linux Kernel Module (C)**  
- **CUDA (C++) with NVCC**  
- **Sysfs + IOCTL Communication**  
- **NVIDIA RTX 3050 GPU (Ampere SM 86)**  

---

## ⚙️ Build & Run

### 1. Build the Kernel Driver
cd driver
make
sudo insmod gpu_trigger_driver.ko

---

## Build the USER App
cd user
nvcc -o user_app user_app.cu -arch=sm_86

---

## Runs in Different Modes
### Mode 0: IRQ-based trigger
sudo ./user_app 0

### Mode 1: sysfs + ioctl
sudo ./user_app 1

### Mode 2: sysfs-triggered GPU computation
sudo ./user_app 2

---

## 📊 Example Output
[CUDA] Device 0: NVIDIA GeForce RTX 3050 6GB Laptop GPU
gpu_trigger_user: running for mode 1
[Driver result] GPU computation done! jiffies=4299973232
[CUDA] N=1048576 sample: C[0]=0.000000, C[1048575]=0.000000

---

## 📂 Directory Structure
gpu_trigger_project/

│── driver/                                           # Kernel driver

│   ├── gpu_trigger_driver.c

│   └── Makefile

│

│── user/                                             # CUDA user app

│   ├── user_app.cu

│   └── c_cpp_properties.json

│

└── README.md

---

## 🚀 Future Work

🔹 Add multi-GPU support.

🔹 Use CUDA events for performance profiling.

🔹 Integrate GPIO interrupts from real hardware.

---

## 👤 Author

Abhinav (@Abhinavcodez)
💻 Linux, CUDA & Systems Programming Enthusiast

---

## ⭐ If you like this project, don’t forget to star the repo on GitHub!

---

## 👉 After adding this, run:

git add README.md

git commit -m "Updated professional README with badges and docs"

git push
