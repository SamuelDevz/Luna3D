# Luna3D

Esta um mini game engine criada a partir de dois repositórios:  "CG" e "Jogos" de JudsonSS. O propósito dessa game engine é para aprender conceitos de computação gráfica e testar algumas APIs gráficas.

## Como construir

### Dependências

Para construir este projeto, você precisa ter instalado as seguintes dependências:

- CMake 3.5 or maior
- Compilador C++ com C++20

#### Windows

- [Windows 10 SDK](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads)

### Configuração

Este projeto usa o CMake para gerar o arquivo de build. Para configurar o projeto, você pode usar os seguintes comandos:

```bash
cmake -B build [flag(s)]
```

### Definições de pré-processador

| Opções do CMake  | Descrição | Valor |
|:----------------:|:------------------------------:|:---:|
| `BUILD_EXAMPLES` | Build examples of the project. | OFF |

### Build

Depois de configurar o projeto, você consegue construir-lo usando os seguintes comandos:

```bash
cmake --build build --config [Release/Debug]
```