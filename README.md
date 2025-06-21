# Luna3D

This a mini game engine created from two repositories: "CG" and "Jogos" by JudsonSS. The purpose is the for learning APIs like Directx and Vulkan.

## How to build

### Dependencies

To build this project, you need to have the following dependencies installed:

- CMake 3.5 or higher
- C++ compiler with C++20

#### Windows

- [Windows SDK](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads)

### Configure

This project uses CMake to generate the build files. To configure the project, you can use
the following commands:

```bash
cmake -B build -S . [flag(s)]
```

### Preprocessor Definitions

| CMake Options    | Description | Value |
|:----------------:|:-----------:|:-----------:|
| `BUILD_EXAMPLES` | Build examples of the project. | OFF |

Alternatively you can set the following preprocessor definitions manually:

| Definition | Description | Value |
|:----------:|:-----------:|:-----:|
| `DIRECTX11` | DirectX 11 | OFF   |

**Nota:** Do you need choose the one API graphics, else the cmake will show you the message of the error.

### Build

After configuring the project, you can build it using the following command:

```bash
cmake --build build
```