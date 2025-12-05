# Parte I. Sincronización con Barrera reutilizable

## Actividad 1: Implementación de la barrera (Monitor) 

Se implementa una **barrera reutilizable** en C utilizando únicamente:
- `pthread_mutex_t` para exclusión mutua.
- `pthread_cond_t` para sincronización de hebras.

La barrera mantiene el siguiente estado interno:
- `int count`: número de hebras que han llegado a la barrera en la etapa actual.  
- `int N`: número total de hebras que deben esperar (parámetro de inicialización).  
- `int etapa`: identificador de la etapa actual.  

Para garantizar correcta coordinación entre hebras:
- `pthread_mutex_t` para proteger el acceso al estado compartido (`count` y `etapa`).  
- `pthread_cond_t` para que las hebras esperen hasta que todas lleguen al punto de sincronización.  

La barrera soporta tres operaciones fundamentales:

1. **Creación e inicialización**  
   - Se inicializan `count = 0`, `N` con el número de hebras, y `etapa = 0`.  
   - Se crean el `pthread_mutex_t` y el `pthread_cond_t`.  

2. **Destrucción**  
   - Se liberan los recursos con `pthread_mutex_destroy` y `pthread_cond_destroy`.  

3. **wait()**  
   - Cada hebra captura la etapa actual en una variable local (`etapaLocal`).  
   - Incrementa `count` al llegar.  
   - Si **no es la última hebra**, se bloquea en `pthread_cond_wait` mientras `etapa` no cambie.  
   - Si **es la última hebra en llegar**:
     1. Resetea `count` a 0.  
     2. Incrementa `etapa`.  
     3. Llama a `pthread_cond_broadcast` para despertar a todas las hebras bloqueadas.  

## Actividad 2: Aplicación de verificación

## Compilación y ejecución
En Linux/WSL:

gcc main.c -o main -pthread

Por defecto:

./main

Con parametros personalizados:
./main N E

# Parte II. Simulador simple de Memoria Virtual

## Actividad 1: Simulador de Traducción de Direcciones
Se implementa un **simulador secuencial de memoria virtual** en C utilizando:
- Operaciones a nivel de bits (`>>`, `&`) para la traducción de direcciones lógico-físicas.
- Algoritmo de reemplazo **Reloj (Clock)** para la gestión de fallos de página.

El simulador mantiene el siguiente estado interno:
- `Marco *marcos`: arreglo de estructuras que representa los marcos de página físicos disponibles.
- `int Nmarcos`: número total de marcos físicos (parámetro de configuración).
- `int clock_hand`: puntero circular que indica el candidato actual para el algoritmo de reemplazo.
- `unsigned long total_referencias` y `total_fallos`: contadores para el cálculo de estadísticas finales.

Para garantizar la correcta simulación de la paginación:
- Se utiliza aritmética de punteros y bits para descomponer la Dirección Virtual (DV) en Número de Página Virtual (VPN) y Offset.
- Se gestiona el bit de `referencia` (uso) dentro de cada `Marco` para decidir qué página expulsar cuando la memoria está llena.

El simulador soporta tres operaciones fundamentales:

1. **Configuración e Inicialización**
   - Se leen los argumentos `Nmarcos` y `tamaño_marco`.
   - Se calculan los bits de desplazamiento `b = log2(tamaño_marco)` y la máscara `MASK` (`2^b - 1`) necesaria para extraer el offset.
   - Se inicializa el arreglo de marcos con `valid = 0` y el puntero del reloj `clock_hand = 0`.

2. **Traducción de Direcciones**
   - Por cada línea del archivo de traza, se lee la dirección virtual (DV) usando `strtoul`.
   - Se calcula el **Offset** (`DV & MASK`) y el **VPN** (`DV >> b`).
   - Se busca linealmente si el VPN ya existe en el arreglo `marcos` (simulación de la TLB/Tabla de Páginas).

3. **Gestión de Fallos y Reemplazo (Clock)**
   - **Caso HIT:** Si el VPN está en memoria, se actualiza el bit de `referencia` a 1 (segunda oportunidad).
   - **Caso FALLO (Memoria con espacio):** Si hay marcos con `valid = 0`, se asigna directamente uno libre.
   - **Caso FALLO (Memoria llena - Algoritmo Reloj):**
     1. Se itera circularmente usando `clock_hand`.
     2. Si el marco apuntado tiene `referencia == 1`, se cambia a 0 y se avanza el puntero.
     3. Si el marco apuntado tiene `referencia == 0`, se selecciona como **víctima**, se reemplaza su contenido con el nuevo VPN, se pone `referencia = 1` y se avanza el puntero.
   - Finalmente, se reconstruye y reporta la Dirección Física (DF) como `(marco << b) | offset`.

## Compilación y ejecución
En Linux/WSL:

gcc sim.c -o sim -lm

Solo resultados:
./sim n_marcos tamano_marco archivo.txt

Completo detalle:
./sim n_marcos tamano_marco --verbose archivo.txt

El archivo debe encontrarse en la misma carpeta que el programa 
