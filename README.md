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

#### Dependências por Interface:
- XLib (X11): libx11-dev, libxfixes-dev, libxcursor-dev, libpng-dev.
- XCB: libxcb1-dev, libxcb-icccm4-dev, libxcb-fixes0-dev, libxcb-errors-dev, libxkbcommon-dev.
- Wayland: libwayland-dev, wayland-protocols, libxkbcommon-dev, zenity.

> Dica de Cursor: O Linux utiliza o formato Xcursor. Você pode converter arquivos .cur do Windows usando a ferramenta win2xcur.

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

### Configuração e Build

1. Clone o repositório.

```bash
https://github.com/SamuelDevz/Luna3D.git
cd Luna3D
```

2. Gere os arquivos de build e compile:

Substitua as flags conforme sua necessidade (veja a tabela abaixo).
```bash
cmake -B build -DBUILD_EXAMPLES=ON -DBUILD_DIRECT3D12=ON
cmake --build build --config Release
```
### Variáveis de Compilação (CMake Flags)

| Opção            | Descrição                          | Padrão |
|------------------|:-----------------------------------|:------:|
| BUILD_EXAMPLES   | Compila os projetos de exemplo.    | OFF    |
| SHARED_LIBRARIES | Compila como libs dinâmicas.       | OFF    |
| BUILD_X11        | Build usando Xlib (Linux).         | OFF    |
| BUILD_XCB        | Build usando XCB (Linux).          | OFF    |
| BUILD_WAYLAND    | Build usando Wayland (Linux).      | OFF    |
| BUILD_VULKAN     | Build usando Vulkan (Win/Linux).   | OFF    |
| BUILD_DIRECT3D11 | Build usando D3D11 (Windows).      | OFF    |
| BUILD_DIRECT3D12 | Build usando D3D12 (Windows).      | OFF    |

## 🤝 Como Contribuir

Contribuições são bem-vindas! Se você tem ideias para novas funcionalidades, melhorias de performance ou correções de bugs, por favor, siga estes passos:

1. Faça um "Fork" do repositório.
2. Crie uma nova branch para sua feature (`git checkout -b feature/minha-feature`).
3. Faça o commit de suas alterações (`git commit -m 'Adiciona minha-feature'`).
4. Envie para a sua branch (`git push origin feature/minha-feature`).
5. Abra um "Pull Request".

## 📄 Licença

Este projeto está licenciado sob a Licença MIT. Veja o arquivo `LICENSE` para mais detalhes.