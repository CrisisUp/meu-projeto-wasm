# Defensor da Terra (Space Invaders WebAssembly)

Um jogo de arcade espacial retrô construído com **C nativo compilado para WebAssembly (Emscripten)** e renderizado a 60 FPS via **HTML5 Canvas** e **Web Audio API**.

---

## 🛠️ Arquitetura do Projeto

O projeto adota uma divisão clara entre lógica de simulação em baixo nível e camada de apresentação no navegador:

```text
Space Invaders/
├── Makefile                 # Script de automação GNU Make
├── src/
│   └── game.c               # Núcleo de física, estados, colisões e controle de memória linear
└── dist/
    ├── index.html           # Renderizador Canvas, áudio sintetizado e camada de apresentação
    └── game.wasm            # Binário WebAssembly compilado a partir do C
```

### 1. Núcleo em C (`src/game.c`)

- **Física com Inércia:** Cálculo de aceleração vetorial e atrito amortecido para a movimentação suave da nave.
- **Gerenciamento de Entidades:** Estruturas de memória compactas para projéteis do jogador (`Bullet`), projéteis hostis (`EnemyBullet`), esquadrões alienígenas (`Enemy`) e a nave-mãe (`Boss`).
- **Detecção de Colisões AABB:** Resolução rápida de interseções no espaço cartesiano sem bibliotecas externas.
- **Memória Linear Compartilhada:** Ponteiros expostos (`EMSCRIPTEN_KEEPALIVE`) lidos diretamente pelo JavaScript via `DataView` sem custo de serialização (zero-copy).

### 2. Camada Web & Visual (`dist/index.html`)

- **Renderização em Canvas:** Sprites em matriz procedural pixel a pixel com rotação de inclinação (*banking*) dinâmica e escala inteira nítida.
- **Tipografia Arcade:** Fonte `'Press Start 2P'` com contorno sólido de alto contraste para máxima legibilidade.
- **Sistemas de Impacto (*Game Juice*):** Efeito de tremor de tela (*screen shake*), rastro contínuo de propulsão nos motores e partículas de explosão com física de dispersão.
- **Áudio Sintetizado (Web Audio API):** Sons gerados dinamicamente via osciladores senoidais, dente-de-serra e ruídos de frequência modulada, sem arquivos de áudio externos.
- **Motor Híbrido de Fallback:** Suporta execução tanto em servidores HTTP tradicionais quanto em ambientes isolados (*sandboxes* ou URLs `blob:`).

---

## 🚀 Requisitos e Ferramentas

- **Emscripten SDK (`emsdk`):** Para compilar o código C em WebAssembly.
- **GNU Make (3.81+):** Para automação da compilação.
- **Python 3:** Para servir localmente os arquivos com cabeçalhos MIME corretos.
- **Navegador Moderno:** Com suporte a WebAssembly e Web Audio API (Chrome, Edge, Firefox, Safari).

---

## 💻 Como Compilar e Executar

### 1. Ativar o Ambiente do Emscripten

No PowerShell:

```powershell
& C:\emsdk\emsdk_env.ps1
```

### 2. Compilar o Projeto

Execute o comando Make na raiz da pasta do projeto:

```powershell
make
```

> Caso queira forçar uma recompilação completa:

```powershell
> make clean
> make
> ```

### 3. Iniciar o Servidor Local

Para disponibilizar o jogo e servir o binário `.wasm`:

```powershell
make serve
```

Abra o navegador no endereço: **`http://localhost:8080`**

---

## 🎮 Controles

| Ação | Teclado |
| :--- | :--- |
| **Mover para a Esquerda** | <kbd>A</kbd> ou <kbd>←</kbd> |
| **Mover para a Direita** | <kbd>D</kbd> ou <kbd>→</kbd> |
| **Disparar Laser** | <kbd>Espaço</kbd> |
| **Iniciar / Reiniciar** | <kbd>Enter</kbd> ou <kbd>Espaço</kbd> |

---

## 👾 Regras e Mecânicas

- **Invasores Padrão:** Cada abate rende **100 pontos**.
- **Perda de Escudos:** Se uma nave inimiga atingir a parte inferior da tela, o jogador perde **1 escudo**.
- **Power-Ups (25% de chance ao abater):**
  - **`3X` (Dourado):** Ativa disparo triplo em leque durante 10 segundos.
  - **`+` (Verde):** Restaura 1 ponto de escudo (máximo de 5).
- **Nave-Mãe Destroyer (Boss):**
  - Surge a partir de **1.500 pontos**.
  - Possui **50 HP** e dispara projéteis de plasma para baixo.
  - Sua derrota concede um bônus de **1.000 pontos**.
- **Recorde (High Score):** Salvo automaticamente no `localStorage` do navegador.

---

## 📦 Layout de Memória Linear (Interoperabilidade C ↔ JS)

| Estrutura | Tamanho por Item | Capacidade | Total |
| :--- | :--- | :--- | :--- |
| `Bullet` | 16 bytes (`float x, y, vx; int active`) | 50 itens | 800 bytes |
| `Enemy` | 16 bytes (`float x, y, speed_y; int active`) | 20 itens | 320 bytes |
| `EnemyBullet` | 20 bytes (`float x, y, vx, vy; int active`) | 20 itens | 400 bytes |
| `Boss` | 28 bytes | 1 item | 28 bytes |
