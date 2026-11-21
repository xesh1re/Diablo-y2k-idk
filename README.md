# 💿 Diablo Glitch 2K

Plugin **VST3** hecho con C++ y [JUCE](https://juce.com) con estética **Y2K / años 2000**
(cromo, vidrio brillante, rosa chicle, burbujas y destellos). Pensado para darle flow
a voces y adlibs: glitch, voz de diablo, robot y más. Todas las perillas tienen
descripción en español: pasa el mouse encima y la barra de abajo te explica qué hace.

## Perillas

| Sección | Perillas | Qué hacen |
|---|---|---|
| **Entrada / Salida** | Entrada, Salida | Volumen antes y después de los efectos |
| **Diablo / Pitch** | Tono, Mezcla Tono, Diablo | Cambio de tono (-12 a +12 semitonos) y capa demoníaca una octava abajo |
| **Glitch** | Stutter, Tamaño Stutter, Chop | Tartamudeo tipo disco rayado y cortes al azar |
| **Robot + Lo-Fi** | Robot, Frec. Robot, Crush | Voz metálica y calidad de celular de los 2000 |
| **Espacio** | Eco Tiempo, Eco Mezcla, Reverb | Eco con repeticiones oscuras y reverb |

## Descargar el plugin ya compilado

1. Sube este proyecto a GitHub (el workflow compila solo).
2. Pestaña **Actions** → último run → baja **DiabloGlitch2K-Windows** de *Artifacts*.
3. Copia la carpeta `Diablo Glitch 2K.vst3` a `C:\Program Files\Common Files\VST3`.
4. En **FL Studio**: *Options → Manage plugins → Find installed plugins*.

> Necesitas FL Studio 20.8 o más nuevo para VST3.

## Compilar en tu PC

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

El `.vst3` queda en `build/DiabloGlitch2K_artefacts/Release/VST3/`.

## Estructura

```
CMakeLists.txt
Source/
  Parameters.h        IDs de los parámetros
  PluginProcessor.*   audio (pitch, stutter, chop, robot, crush, eco, reverb)
  PluginEditor.*      interfaz Y2K
.github/workflows/build.yml
```
