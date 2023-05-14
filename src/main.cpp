#include <Wire.h>
#include <Keypad.h>
#include <iterator>
#include <melody_player.h>
#include <melody_factory.h>
#include "U8g2lib.h"
#include <Preferences.h>

#define OLED_SDA 22
#define OLED_SCL 23
//#define OLED_RST 16
#define BUZZER_PIN 21

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
};

// For ESP32 Microcontroller
byte rowPins[ROWS] = {14, 27, 26, 25};
byte colPins[COLS] = {16, 17, 5, 18};

Preferences preferences;

U8G2_ST7567_ENH_DG128064I_F_SW_I2C display(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
MelodyPlayer player(BUZZER_PIN, 0, LOW);

char texto[20];

int op1;
int op2;
int num;
int numDigitos;
int maxDigitos;

int puntos = 0;
int record = 0;

int tablas[10];
int resultado[10][10] = {};

int tiempo = 0;
int tipo = 0;
int tipoActual = 0;
int configurando = 1;
int secuencia = 0;
int tiempoRestante = 0;

void reiniciar() {
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            resultado[i][j] = 0;
        }
    }
}

void nuevaPrueba() {
    tiempoRestante = tiempo;

    int solucion;
    if (tipo == 3) {
        tipoActual = random(2);
    } else if (tipo == 4) {
        tipoActual = random(3);
    } else {
        tipoActual = tipo;
    }
    int total = 0;
    for (int i = 0; i < 10; ++i) {
        if (tablas[i] == 0) {
            for (int j = 0; j < 10; ++j) {
                total += (resultado[i][j] == 0) ? 1 : 0;
            }
        }
    }
    if (total == 0) {
        reiniciar();
    }
    switch (tipoActual) {
        // A x B == C
        case 0:
            // A: Seleccionar una tabla permitida al azar
            do {
                op1 = random(1, 11);
            } while (tablas[op1 - 1] == 1);
            // B: Elegir un número del 1 al 10, salvo para
            // la tabla del 10, que el máximo es 9
            // (evitar los números acertados)
            do {
                op2 = random(1, op1 == 10 ? 10 : 11);
            } while (resultado[op1 - 1][op2 - 1] == 1);
            // C es la solución a averiguar
            solucion = op1 * op2;
            break;
        case 1:
            // A: Seleccionar una tabla permitida al azar
            do {
                op1 = random(1, 11);
            } while (tablas[op1 - 1] == 1);
            // B: es la solución a averiguar. Un número del 1 al 10,
            // salvo para la tabla del 10, que el máximo es 9
            do {
                solucion = random(1, op1 == 10 ? 10 : 11);
                // C: Resultado de multiplicar A por B
                op2 = op1 * solucion;
            } while (resultado[op1 - 1][solucion - 1] == 1);
            break;
        default:
            // A: Es la solución a averiguar. Un número del 1 al 10
            do {
                do {
                    solucion = random(1, 11);
                } while (tablas[solucion - 1] == 1);
                // B: Elegir un número del 1 al 10, salvo para
                // la tabla del 10, que el máximo es 9
                op1 = random(1, solucion == 10 ? 10 : 11);
                // C: Resultado de multiplicar A por B
                op2 = op1 * solucion;
            } while (resultado[op1 - 1][solucion - 1] == 1);
            break;
    }
    maxDigitos = solucion < 10 ? 1 : 2;
    if (solucion >= 100) maxDigitos++;

    numDigitos = 0;
    num = 0;
}

void procesaSecuencia(char key) {
    switch (secuencia) {
        case 0:
            secuencia = (key == 'A') ? 1 : 0;
            break;
        case 1:
            secuencia = (key == 'C') ? 2 : 0;
            break;
        case 2:
            secuencia = (key == 'A') ? 3 : 0;
            break;
        case 3:
            secuencia = (key == 'B') ? 4 : 0;
            break;
        case 4:
            configurando = (key == 'A') ? 1 : 0;
            secuencia = 0;
            break;
        default:
            secuencia = 0;
    }
    /*char texto2[100];
    display.setFont(u8g2_font_profont10_mf);
    sprintf(texto2, "%d", secuencia);
    display.drawStr(120, 63, texto2);
    display.sendBuffer();*/
}

void dibujarPuntos() {
    char texto2[100];
    display.setFont(u8g2_font_helvB14_tf);
    sprintf(texto2, "Ptos %d / %d", puntos, record);
    display.drawStr(0, 63, texto2);
}

void ganar() {
    const char* winMelody = "start:d=4,o=6,b=200: 8c, 8e, 4g";

    puntos++;
    display.clearBuffer();
    display.setFont(u8g2_font_helvB18_tf);
    display.drawUTF8(0, 31, "¡Bien!");
    dibujarPuntos();
    display.sendBuffer();

    Melody melody = MelodyFactory.loadRtttlString(winMelody);
    player.play(melody);
}

void compruebaDelay(int delayOrig) {
    for (int j = 0; j < delayOrig / 50; j++) {
        char key = keypad.getKey();
        if (key != 0) procesaSecuencia(key);
        delay(50);
    }
}

void perder() {
    const char* loseMelody = "start:d=4,o=4,b=100: 1c";
    const char* recordMelody = "start:d=4,o=4,b=300: 8c,8e,8g,4c5,8p,8g,4c5";

    char texto2[20];

    switch (tipoActual) {
        case 0:
            sprintf(texto, "%dx%d<>%d", op1, op2, num);
            sprintf(texto2, "%dx%d=%d", op1, op2, op1 * op2);
            break;
        case 1:
            sprintf(texto, "%dx%d<>%d", op1, num, op2);
            sprintf(texto2, "%dx%d=%d", op1, op2 / op1, op2);
            break;
        case 2:
            sprintf(texto, "%dx%d<>%d", num, op1, op2);
            sprintf(texto2, "%dx%d=%d", op2 / op1, op1, op2);
            break;
    }

    if (tiempoRestante < 0) {
        strcpy(texto, texto2);
    }

    //display.invertDisplay();
    display.clearBuffer();
    display.setFont(u8g2_font_helvB18_tf);
    display.drawStr(0, 31, texto);
    dibujarPuntos();
    display.sendBuffer();

    Melody melody = MelodyFactory.loadRtttlString(loseMelody);
    player.play(melody);

    for (int i = 0; i < 3; i++) {
        //display.invertDisplay();
        display.clearBuffer();
        display.setFont(u8g2_font_helvB18_tf);
        display.drawStr(0, 31, texto);
        dibujarPuntos();
        display.sendBuffer();
        compruebaDelay(750);
        //display.normalDisplay();
        display.clearBuffer();
        display.setFont(u8g2_font_helvB18_tf);
        display.drawStr(0, 31, texto2);
        dibujarPuntos();
        display.sendBuffer();
        compruebaDelay(750);
    }

    display.setFont(u8g2_font_logisoso24_tf);
    sprintf(texto2, "Ptos. %d", puntos);
    display.clearBuffer();
    display.drawStr(0, 31, texto2);
    if (puntos > record) {
        display.drawStr(0, 63, "¡Record!");
        display.sendBuffer();
        record = puntos;
        melody = MelodyFactory.loadRtttlString(recordMelody);
        player.play(melody);
        preferences.begin("app", false);
        preferences.putInt("record", record);
        preferences.end();
    } else {
        display.setFont(u8g2_font_helvB14_tf);
        sprintf(texto2, "Récord: %d", record);
        display.drawUTF8(0, 63, texto2);
        display.sendBuffer();
    }

    compruebaDelay(1000);

    display.setFont(u8g2_font_helvB18_tf);

    puntos = 0;
    reiniciar();
}

void comprobar() {
    int haGanado;
    switch (tipoActual) {
        case 1:
            haGanado = op1 * num == op2;
            if (haGanado) {
                resultado[op1 - 1][num - 1] = 1;
            }
            break;
        case 2:
            haGanado = num * op1 == op2;
            if (haGanado) {
                resultado[num - 1][op1 - 1] = 1;
            }
            break;
        default:
            haGanado = op1 * op2 == num;
            if (haGanado) {
                resultado[op1 - 1][op2 - 1] = 1;
            }
            break;
    }
    if (haGanado) {
        ganar();
    } else {
        perder();
    }
    nuevaPrueba();
}

void leerPreferencias() {
    preferences.begin("app", false);
    record = preferences.getInt("record", 0);
    tiempo = preferences.getInt("tiempo", 0);
    tipo = preferences.getInt("tipo", 0);
    for (int i = 0; i < 10; i++) {
        char nombre[9] = "tablas";
        itoa(i + 1, &nombre[6], 10);
        tablas[i] = preferences.getInt(nombre, 0);
    }
    preferences.end();
}

void guardarPreferencias() {
    preferences.begin("app", false);
    preferences.putInt("tipo", tipo);
    preferences.putInt("tiempo", tiempo);
    preferences.putInt("record", record);
    for (int i = 0; i < 10; i++) {
        char nombre[9] = "tablas";
        itoa(i + 1, &nombre[6], 10);
        preferences.putInt(nombre, tablas[i]);
    }
    preferences.end();
}

void setup() {
#ifdef OLED_RST
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, HIGH);
#endif
    display.enableUTF8Print();

    if (keypad.isPressed('A')) configurando = 1;

    leerPreferencias();

    display.setI2CAddress(0x3F * 2);
    display.enableUTF8Print();
    display.begin();
    display.clearBuffer();
    display.sendBuffer();

    randomSeed(analogRead(0));
    nuevaPrueba();

    const char* startMelody = "aadams:d=4,o=5,b=180:8c,f,8a,f,8c,b4,2g,8f,e,8g,e,8e4,a4,2f,8c,f,8a,f,8c,b4,2g,8f,e,8c,d,8e,1f,8c,8d,8e,8f,1p,8d,8e,8f#,8g,1p,8d,8e,8f#,8g,p,8d,8e,8f#,8g,p,8c,8d,8e,8f";

    display.clearBuffer();
    display.setFont(u8g2_font_helvB18_tf);
    display.drawUTF8(0, 21, "¡Bienvenido");
    display.drawStr(0, 42, "a las tablas de");
    display.drawStr(0, 63, "multiplicar!");
    display.sendBuffer();

    Melody melody = MelodyFactory.loadRtttlString(startMelody);
    player.playAsync(melody);
    while (keypad.getKey() == 0 && player.isPlaying()) {
        delay(10);
    }
    player.stop();
    while (keypad.getKey() != 0) {
        delay(10);
    }
}

const char* click = "start:d=32,o=4,b=800: g,p";
Melody clickMelody = MelodyFactory.loadRtttlString(click);

void dibujarTiempo() {
    if (tiempo == 0) return;
    int altura = 64 * tiempoRestante / tiempo;
    display.drawBox(124, 64 - altura, 4, altura);
}

void dibujarTexto() {
    display.clearBuffer();
    display.setFont(u8g2_font_helvB18_tf);

    switch (tipoActual) {
        case 0:
            if (numDigitos == 0) {
                sprintf(texto, "%dx%d=?", op1, op2);
            } else {
                sprintf(texto, "%dx%d=%d", op1, op2, num);
            }
            if (maxDigitos > 1 && num < 10) strcat(texto, "?");
            if (maxDigitos > 2 && num < 100) strcat(texto, "?");
            break;
        case 1:
            if (numDigitos == 0) {
                sprintf(texto, "%dx?", op1);
            } else {
                sprintf(texto, "%dx%d", op1, num);
            }
            if (maxDigitos > 1 && num < 10) strcat(texto, "?");
            if (maxDigitos > 2 && num < 100) strcat(texto, "?");
            sprintf(&texto[strlen(texto)], "=%d", op2);
            break;
        case 2:
            if (numDigitos == 0) {
                strcpy(texto, "?");
            } else {
                sprintf(texto, "%d", num);
            }
            if (maxDigitos > 1 && num < 10) strcat(texto, "?");
            if (maxDigitos > 2 && num < 100) strcat(texto, "?");
            sprintf(&texto[strlen(texto)], "x%d=%d", op1, op2);
            break;
    }
    display.drawStr(0, 31, texto);
    dibujarPuntos();
    dibujarTiempo();
    display.sendBuffer();
}

void dibujarConfiguracion() {
    display.clearBuffer();
    display.setFont(u8g2_font_logisoso16_tf);
    char cadena[20];
    for (int i = 0; i < 10; i++) {
        itoa(i + 1, cadena, 10);
        if (tablas[i] == 0) display.drawStr(12 * i, 16, cadena);
    }
    switch (tipo) {
        case 0:
            display.drawStr(0, 33, "AxB=?");
            break;
        case 1:
            display.drawStr(0, 33, "Ax?=C");
            break;
        case 2:
            display.drawStr(0, 33, "?xB=C");
            break;
        case 3:
            display.drawStr(0, 33, "Ax?=?");
            break;
        case 4:
            display.drawStr(0, 33, "?x?=?");
            break;
    }
    if (tiempo == 0) {
        display.drawStr(72, 33, "-- s.");
    } else {
        sprintf(cadena, "%d s.", tiempo / 1000);
        display.drawStr(72, 33, cadena);
    }
    display.setFont(u8g2_font_profont10_mf);
    display.drawUTF8(0, 49, "A] Tipo B] Tiempo C] Salir");
    display.drawUTF8(0, 61, "D] Guardar *] Récord a 0");
    display.sendBuffer();
}

void loop() {
    char key;
    int ultAltura = -1;

    if (configurando == 0) {
        dibujarTexto();
        while ((key = keypad.getKey()) == 0) {
            if (tiempo > 0) {
                int altura = 64 * tiempoRestante / tiempo;
                if (altura != ultAltura) {
                    dibujarTexto();
                    ultAltura = altura;
                }
                tiempoRestante -= 50;
                if (tiempoRestante < 0) break;
            }
            delay(50);
        }

        if (tiempoRestante < 0) {
            perder();
            nuevaPrueba();
            return;
        }

        if (key == '*' && numDigitos > 0) {
            numDigitos--;
            num = num / 10;
        } else if (key >= '0' && key <= '9' && numDigitos < maxDigitos) {
            if (numDigitos != 0 || key != '0') {
                num = num * 10 + key - '0';
                numDigitos++;
                player.play(clickMelody);
                if (numDigitos == maxDigitos) {
                    dibujarTexto();
                    delay(200);
                    comprobar();
                    return;
                }
            }
        }
        procesaSecuencia(key);
        while (keypad.getKey() == key) delay(50);
    } else {
        dibujarConfiguracion();
        while ((key = keypad.getKey()) == 0) delay(50);
        if (key >= '0' && key <= '9') {
            int indice = key - '0' - 1;
            if (indice < 0) indice = 9;
            // Impedir quitar todas
            int cuenta = 0;
            for (int i = 0; i < 10; i++) {
                cuenta += tablas[i];
            }
            if (cuenta < 9 || tablas[indice] == 1) {
                tablas[indice] = 1 - tablas[indice];
            }
            reiniciar();
        } else if (key == 'A') {
            tipo = (tipo + 1) % 5;
            reiniciar();
        } else if (key == 'B') {
            tiempo = tiempo == 0 ? 10000 : tiempo - 1000;
            tiempoRestante = tiempo;
        } else if (key == 'C') {
            configurando = 0;
        } else if (key == 'D') {
            guardarPreferencias();
            configurando = 0;
        } else if (key == '*') {
            record = 0;
        }
        while (keypad.getKey() == key) delay(50);
    }
}
