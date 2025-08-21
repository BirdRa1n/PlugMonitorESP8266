#include <Arduino.h>

// ===== Configurações do hardware =====
const int PIN_ACS = A0;     // D1 Mini: ADC único
const float ADC_VMAX = 3.3; // máximo do ADC A0
const int ADC_RES = 1024;   // resolução 10 bits

// ===== ACS712 =====
const float SENS_V_PER_A = 0.100;    // 20A → 0.100 V/A
const float OUT_SCALING = 3.3 / 5.0; // divisor 5V → 3.3V
const float SENS_EFFECTIVE = SENS_V_PER_A * OUT_SCALING;

// ===== Rede elétrica =====
const float MAINS_V = 220.0;    // tensão nominal
const float PF = 0.90;          // fator de potência estimado
const float MAINS_FREQ = 60.0;  // Hz
const int CYCLES_FAST = 5;      // ciclos p/ leitura rápida (~80-100 ms)
const int SAMPLE_DELAY_US = 50; // intervalo entre amostras

// ===== Energia =====
unsigned long lastMs = 0;
float energy_kWh = 0.0;

// ===== Calibração =====
float offsetVolts = 0.0;
float noise_Irms = 0.0;

// ===== Util =====
inline float adcToVolts(int adc)
{
  return (adc * ADC_VMAX) / (ADC_RES - 1);
}

float measureOffset_ms(unsigned long ms)
{
  unsigned long t0 = millis();
  double acc = 0;
  uint32_t n = 0;
  while (millis() - t0 < ms)
  {
    acc += adcToVolts(analogRead(PIN_ACS));
    n++;
    delayMicroseconds(50);
  }
  return n ? (acc / n) : (ADC_VMAX / 2.0);
}

float measureIrms_cycles_fast(int cycles, float centerVolts)
{
  const unsigned long duration_us = (unsigned long)((cycles * 1e6) / MAINS_FREQ);
  unsigned long t0 = micros();
  double accSq = 0;
  uint32_t n = 0;

  while ((micros() - t0) < duration_us)
  {
    float v = adcToVolts(analogRead(PIN_ACS));
    float v_ac = v - centerVolts;
    float i = v_ac / SENS_EFFECTIVE;
    accSq += i * i;
    n++;
    delayMicroseconds(SAMPLE_DELAY_US);
  }

  return n ? sqrt(accSq / n) : 0.0;
}

void quickBaseline()
{
  // 1) Offset real do sensor
  offsetVolts = measureOffset_ms(500);
  // 2) Ruído de fundo (Irms sem carga)
  noise_Irms = measureIrms_cycles_fast(5, offsetVolts);

  Serial.print("Offset(V): ");
  Serial.print(offsetVolts, 4);
  Serial.print(" | Noise Irms(A): ");
  Serial.println(noise_Irms, 4);
}

void setup()
{
  Serial.begin(115200);
  delay(800);
  Serial.println("\nMedidor de Consumo - D1 Mini + ACS712 (RMS rápido, kWh)");
  Serial.println("Calibrando sensor...");
  quickBaseline();
  lastMs = millis();
}

void loop()
{
  // Ajuste lento do offset (drift térmico)
  float newOffset = measureOffset_ms(50);
  offsetVolts = 0.98f * offsetVolts + 0.02f * newOffset;

  // Leitura RMS rápida
  float Irms_meas = measureIrms_cycles_fast(CYCLES_FAST, offsetVolts);

  // Remove ruído de fundo (linear, não zera pequenas correntes)
  float Irms = (Irms_meas > noise_Irms) ? (Irms_meas - noise_Irms) : 0.0;

  // Potência ativa (W)
  float P_W = MAINS_V * Irms * PF;

  // Energia acumulada (kWh)
  unsigned long now = millis();
  float hours = (now - lastMs) / 3600000.0f;
  lastMs = now;
  energy_kWh += (P_W * hours) / 1000.0f;

  // Saída Serial
  Serial.print("I_RMS: ");
  Serial.print(Irms, 3);
  Serial.print(" A | P: ");
  Serial.print(P_W, 1);
  Serial.print(" W | E: ");
  Serial.print(energy_kWh, 6);
  Serial.println(" kWh");

  delay(100); // ~10 leituras por segundo
}