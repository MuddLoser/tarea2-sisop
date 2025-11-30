# Parte I. Sincronización con Barrera reutilizable

# Actividad 1: Implementación de la barrera (Monitor) 

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

# Actividad 2: Aplicación de verificación

## Compilación y ejecución
En Linux/WSL:

gcc main.c -o main -pthread

Por defecto:

./main

Con parametros personalizados:
./main N E
