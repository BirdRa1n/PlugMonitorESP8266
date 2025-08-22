# PlugMonitorESP8266

Um medidor de consumo elétrico em tempo real usando ESP8266 (D1 Mini) e sensor de corrente ACS712.

## 📋 Descrição

Este projeto implementa um monitor de consumo elétrico que mede corrente RMS, potência ativa e energia acumulada (kWh) usando um microcontrolador ESP8266 e sensor de corrente ACS712. O sistema oferece calibração automática, compensação de ruído e configuração via porta serial.

> **🖥️ Interface Desktop**: Este firmware funciona em conjunto com o aplicativo desktop **PlugMonitor** desenvolvido em Electron para visualização e monitoramento dos dados em tempo real. Para a interface completa do usuário, consulte: [PlugMonitor](https://github.com/BirdRa1n/PlugMonitor)

## ⚡ Características

- **Medição em tempo real**: Corrente RMS, potência ativa e energia acumulada
- **Calibração automática**: Compensação de offset e ruído de fundo
- **Configuração flexível**: Parâmetros ajustáveis via porta serial
- **Medição rápida**: ~10 leituras por segundo com algoritmo otimizado
- **Compensação térmica**: Ajuste automático de drift térmico do sensor
- **Interface serial**: Comandos para configuração e monitoramento

## 🔧 Hardware Necessário

- **ESP8266 D1 Mini Lite**
- **Sensor de corrente ACS712** (5A, 20A ou 30A)
- **Divisor de tensão** (5V → 3.3V) para compatibilidade com ESP8266
- **Fonte de alimentação** 3.3V/5V

### Conexões

```
ACS712 → D1 Mini
VCC    → 5V (se disponível) ou 3.3V
GND    → GND
OUT    → A0 (através de divisor de tensão 5V→3.3V)
```

## 🏗️ Arquitetura do Sistema Completo

Este firmware ESP8266 é parte de um sistema completo de monitoramento elétrico:

```
┌─────────────────┐    Serial/USB    ┌──────────────────┐
│  ESP8266 + ACS712│ ──────────────► │ Aplicativo Desktop│
│  (Hardware)     │                 │   PlugMonitor    │
│                 │ ◄────────────── │   (Electron)     │
│ • Mede corrente │    Comandos     │                  │
│ • Calcula potência│                │ • Interface gráfica│
│ • Envia dados   │                 │ • Configuração   │
│ • Recebe config │                 │ • Visualização   │
└─────────────────┘                 └──────────────────┘
```

**Fluxo de Dados:**
1. ESP8266 mede corrente via ACS712
2. Calcula potência e energia acumulada
3. Envia dados via serial: `I_RMS: X.XXX A | P: XXX.X W | E: X.XXXXXX kWh`
4. Aplicativo desktop recebe e exibe os dados
5. Usuário pode configurar parâmetros via aplicativo
6. Comandos são enviados de volta ao ESP8266

## 📦 Instalação

### Pré-requisitos

- [PlatformIO](https://platformio.org/) instalado
- Driver USB para ESP8266
- **Aplicativo PlugMonitor** instalado para interface completa (consulte [README.md](https://github.com/BirdRa1n/PlugMonitor))

### Passos

1. Clone o repositório:
```bash
git clone https://github.com/birdra1n/PlugMonitorESP8266.git
cd PlugMonitorESP8266
```

2. Compile e faça upload:
```bash
pio run --target upload
```

3. Abra o monitor serial:
```bash
pio device monitor
```

## 🚀 Uso

### Setup Completo do Sistema

Para usar o sistema completo PlugMonitor:

1. **Configure o Hardware** (este projeto):
   - Monte o circuito ESP8266 + ACS712
   - Faça upload do firmware
   - Conecte via USB ao computador

2. **Instale o Software Desktop**:
   - Baixe e instale o aplicativo PlugMonitor ([README.md]([README.md](https://github.com/BirdRa1n/PlugMonitor)))
   - Execute o aplicativo
   - Selecione a porta serial do ESP8266
   - Comece o monitoramento!

### Monitoramento Básico

Após o upload, o sistema iniciará automaticamente a calibração e começará a exibir as medições:

```
Medidor de Consumo - D1 Mini + ACS712 (RMS rápido, kWh)
Calibrando sensor...
Offset(V): 1.6500 | Noise Irms(A): 0.0120
I_RMS: 2.450 A | P: 485.1 W | E: 0.000125 kWh
```

### Configuração via Serial

O sistema aceita comandos via porta serial no formato `>COMANDO,VALOR`:

#### Comandos Disponíveis

| Comando | Descrição | Exemplo |
|---------|-----------|---------|
| `SET_V` | Define tensão da rede (V) | `>SET_V,220.0` |
| `SET_PF` | Define fator de potência | `>SET_PF,0.95` |
| `SET_SENS` | Define sensibilidade do ACS712 (V/A) | `>SET_SENS,0.100` |
| `GET_CONFIG` | Mostra configuração atual | `>GET_CONFIG` |

#### Exemplos de Uso

```bash
# Configurar tensão para 127V
>SET_V,127.0

# Configurar fator de potência para 0.95
>SET_PF,0.95

# Configurar sensor ACS712-05A (0.185 V/A)
>SET_SENS,0.185

# Verificar configuração
>GET_CONFIG
```

### Sensibilidades do ACS712

| Modelo | Corrente Máxima | Sensibilidade (V/A) |
|--------|-----------------|-------------------|
| ACS712-05A | 5A | 0.185 |
| ACS712-20A | 20A | 0.100 |
| ACS712-30A | 30A | 0.066 |

## 📊 Saída de Dados

O sistema fornece dados em tempo real via porta serial:

```
I_RMS: 2.450 A | P: 485.1 W | E: 0.000125 kWh
```

Onde:
- **I_RMS**: Corrente eficaz em Ampères
- **P**: Potência ativa em Watts
- **E**: Energia acumulada em kWh

## ⚙️ Configurações Padrão

```cpp
// Rede elétrica
MAINS_V = 220.0V        // Tensão nominal
PF = 0.90               // Fator de potência
MAINS_FREQ = 60.0Hz     // Frequência da rede

// Sensor ACS712-20A
SENS_V_PER_A = 0.100    // Sensibilidade (V/A)

// Medição
CYCLES_FAST = 5         // Ciclos para leitura rápida
SAMPLE_DELAY_US = 50    // Intervalo entre amostras
```

## 🔬 Algoritmo de Medição

1. **Calibração inicial**: Mede offset do sensor e ruído de fundo
2. **Compensação térmica**: Ajuste contínuo do offset (filtro passa-baixa)
3. **Medição RMS**: Amostragem durante 5 ciclos da rede (~83ms @ 60Hz)
4. **Filtragem de ruído**: Subtração do ruído de fundo medido
5. **Cálculo de potência**: P = V × I × FP
6. **Integração de energia**: Acumulação temporal em kWh

## 🛠️ Desenvolvimento

### Estrutura do Projeto

```
PlugMonitorESP8266/
├── src/
│   └── main.cpp          # Código principal
├── include/              # Headers (vazio)
├── lib/                  # Bibliotecas locais (vazio)
├── test/                 # Testes (vazio)
├── platformio.ini        # Configuração PlatformIO
└── README.md            # Este arquivo
```

### Compilação

```bash
# Compilar
pio run

# Upload
pio run --target upload

# Monitor serial
pio device monitor

# Limpeza
pio run --target clean
```

## 📈 Melhorias Futuras

- [ ] Interface web para monitoramento remoto
- [ ] Logging de dados em cartão SD
- [ ] Integração com MQTT/IoT
- [ ] Display OLED para visualização local
- [ ] Múltiplos canais de medição
- [ ] Calibração automática com carga conhecida

## 🤝 Contribuição

1. Faça um fork do projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

## 👨‍💻 Autor

**Dário Jr** - [GitHub](https://github.com/birdra1n)

## 🙏 Agradecimentos

- Comunidade Arduino/ESP8266
- Documentação do sensor ACS712
- Contribuidores do PlatformIO

---

⭐ Se este projeto foi útil para você, considere dar uma estrela!
