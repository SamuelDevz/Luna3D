# Luna3D

Esta um mini game engine criada a partir de dois repositórios: "CG" e "Jogos" de JudsonSS. O propósito dessa game engine é para aprender conceitos de computação gráfica e algumas APIs como de gráficos e janelas por exemplo.

Obviamente que não há pretenção em concorrer com motores profissionais a nível de otimização, facilidade de uso ou disponibilidade de ferramentas.

## 🏛️ Arquitetura do Projeto

```
Luna3D/
├── src/                # Código-fonte (.cpp)
│   ├── win/            # plataforma windows
│   └── linux/          # plataforma linux
│       ├── XCB/        # biblioteca XCB
│       ├── Xlib/       # biblioteca Xlib
│       └── Wayland/    # biblioteca Wayland
├── examples/           # Alguns exemplos de código usando a game engine
│   ├── simplewindow/   # Cria um simples janela
│   ├── hellotriangle/  # Cria um simples triangle na tela
│   └── ...             # Outros exemplos (Em breve)
├── build/              # Diretório de saída da compilação (ignorado pelo Git)
├── CMakeLists.txt      # Automatiza o processo de build
└── README.md           # Esta documentação
```

## Como construir

### Dependências

Para construir este projeto, você precisa ter instalado as seguintes dependências:

- CMake 3.5 or maior
- Compilador C++ com C++20

### Windows

- [Windows 10 SDK](https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads)
- Graphics Tools

### Linux

#### XLib

- X11
- x11-fixes
- Xcursor
- png

#### XCB

- xcb
- x11-xcb
- xcb-fixes
- xcb-icccm
- Xcursor
- png
- xkbcommon
- xcb-errors

**OBS:** Tu precisa do arquivo no formato Xcursor para mudar o cursor da janela. Uma dica seria pegar um arquivo .CUR e converte-lo usando [win2xcur](https://github.com/quantum5/win2xcur)

#### Wayland

- wayland-client
- wayland-cursor
- wayland-protocols
- pkg-config
- xkbcommon
- zenity

### Vulkan

- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
    - GLM Headers

### Configuração e Construção do projeto

1. Clone o repositório.

```bash
https://github.com/SamuelDevz/Luna3D.git
cd Luna3D
```

2. Construa o projeto usando os comandos do cmake.

```bash
cmake -B build [flag(s)]
cmake --build build [Release/Debug]
```

### Definições de pré-processador

| Opções do CMake    | Descrição                           | Valor|
|:------------------:|:-----------------------------------:|:----:|
| `BUILD_EXAMPLES`   | Build examples of the project.      | OFF  |
| `SHARED_LIBRARIES` | Use the shared libs in the project. | OFF  |

| Bibliotecas        | Plataforma      | Descrição                           | Valor |
|:------------------:|:---------------:|:-----------------------------------:|:-----:|
| `BUILD_X11`        | Linux           | Build the engine using Xlib.        | OFF   |
| `BUILD_XCB`        | Linux           | Build the engine using XCB.         | OFF   |
| `BUILD_WAYLAND`    | Linux           | Build the engine using Wayland.     | OFF   |
| `BUILD_DIRECT3D11` | Windows         | Build the engine using Direct3D 11. | OFF   |
| `BUILD_DIRECT3D12` | Windows         | Build the engine using Direct3D 12. | OFF   |
| `BUILD_VULKAN`     | Windows / Linux | Build the engine using Vulkan.      | OFF   |

Para habilitar esses definições você deve habilitar no local onde está as flag(s) acima. Aqui um exemplo:

```bash
cmake -B build -DBUILD_EXAMPLES=ON -DBUILD_DIRECT3D12=ON
cmake --build build
```

## 🤝 Como Contribuir

Contribuições são bem-vindas! Se você tem ideias para novas funcionalidades, melhorias de performance ou correções de bugs, por favor, siga estes passos:

1. Faça um "Fork" do repositório.
2. Crie uma nova branch para sua feature (`git checkout -b feature/minha-feature`).
3. Faça o commit de suas alterações (`git commit -m 'Adiciona minha-feature'`).
4. Envie para a sua branch (`git push origin feature/minha-feature`).
5. Abra um "Pull Request".

## 📄 Licença

Este projeto está licenciado sob a Licença MIT. Veja o arquivo `LICENSE` para mais detalhes.